#!/usr/bin/env python3
"""
ESP32 to Fomalhaut Bridge
=========================
Script puente que lee logs del ESP32 por puerto serial y los envía al servidor
local Java-Spring de Fomalhaut.

Autor: TeideSat
Fecha: Diciembre 2025
"""

import serial
import requests
import json
import time
import re
from datetime import datetime
from typing import Dict, Optional
import sys

class ESP32Bridge:
    def __init__(self, config_path: str = "config.json"):
        """Inicializa el bridge con la configuración especificada"""
        self.config = self.load_config(config_path)
        self.serial_port = None
        self.session = requests.Session()
        self.stats = {
            'lines_read': 0,
            'packets_sent': 0,
            'errors': 0
        }
        
    def load_config(self, config_path: str) -> Dict:
        """Carga la configuración desde un archivo JSON"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            print(f"❌ Error: No se encontró el archivo {config_path}")
            print("📝 Creando configuración por defecto...")
            default_config = {
                "serial": {
                    "port": "/dev/ttyUSB0",
                    "baudrate": 115200,
                    "timeout": 1
                },
                "server": {
                    "base_url": "http://localhost:8080",
                    "endpoints": {
                        "telemetry": "/api/telemetry",
                        "system": "/api/telemetry/system",
                        "power": "/api/telemetry/power",
                        "temperature": "/api/telemetry/temperature",
                        "comms": "/api/telemetry/comms"
                    }
                },
                "debug": True,
                "batch_size": 10,
                "retry_attempts": 3
            }
            with open(config_path, 'w') as f:
                json.dump(default_config, f, indent=2)
            return default_config
    
    def connect_serial(self) -> bool:
        """Conecta al puerto serial del ESP32"""
        try:
            self.serial_port = serial.Serial(
                port=self.config['serial']['port'],
                baudrate=self.config['serial']['baudrate'],
                timeout=self.config['serial']['timeout']
            )
            print(f"✅ Conectado a {self.config['serial']['port']} @ {self.config['serial']['baudrate']} baud")
            return True
        except serial.SerialException as e:
            print(f"❌ Error al conectar al puerto serial: {e}")
            print(f"💡 Verifica que el ESP32 está conectado en {self.config['serial']['port']}")
            return False
    
    def parse_log_line(self, line: str) -> Optional[Dict]:
        """
        Parsea una línea de log del ESP32 y extrae información estructurada
        
        Ejemplos de formatos esperados:
        - [Sistema] CPU: 45%, RAM: 234KB
        - [Potencia] Voltaje: 3.7V, Corriente: 0.5A
        - [Temperatura] Sensor1: 25.3°C
        - [Comms] RSSI: -65dBm, SNR: 8dB
        """
        line = line.strip()
        if not line or line.startswith('---') or line.startswith('==='):
            return None
        
        timestamp = datetime.now().isoformat()
        
        # Detectar tipo de telemetría por patrones
        telemetry_data = {
            'timestamp': timestamp,
            'raw_line': line,
            'type': 'general'
        }
        
        # Sistema: CPU, RAM, Stack
        if 'CPU' in line or 'RAM' in line or 'Stack' in line:
            telemetry_data['type'] = 'system'
            cpu_match = re.search(r'CPU[:\s]+(\d+)%', line)
            ram_match = re.search(r'RAM[:\s]+(\d+)', line)
            if cpu_match:
                telemetry_data['cpu_usage'] = int(cpu_match.group(1))
            if ram_match:
                telemetry_data['ram_free'] = int(ram_match.group(1))
        
        # Potencia: Voltaje, Corriente
        elif 'Voltaje' in line or 'Voltage' in line or 'Corriente' in line or 'Current' in line:
            telemetry_data['type'] = 'power'
            voltage_match = re.search(r'Voltaje[:\s]+([\d.]+)', line)
            current_match = re.search(r'Corriente[:\s]+([\d.]+)', line)
            if voltage_match:
                telemetry_data['voltage'] = float(voltage_match.group(1))
            if current_match:
                telemetry_data['current'] = float(current_match.group(1))
        
        # Temperatura
        elif 'Temp' in line or '°C' in line:
            telemetry_data['type'] = 'temperature'
            temp_match = re.search(r'([\d.]+)\s*°?C', line)
            if temp_match:
                telemetry_data['temperature'] = float(temp_match.group(1))
        
        # Comunicaciones: RSSI, SNR, Paquetes
        elif 'RSSI' in line or 'SNR' in line or 'Paquetes' in line or 'Packets' in line:
            telemetry_data['type'] = 'comms'
            rssi_match = re.search(r'RSSI[:\s]+(-?\d+)', line)
            snr_match = re.search(r'SNR[:\s]+(-?\d+)', line)
            if rssi_match:
                telemetry_data['rssi'] = int(rssi_match.group(1))
            if snr_match:
                telemetry_data['snr'] = int(snr_match.group(1))
        
        return telemetry_data
    
    def send_to_server(self, data: Dict) -> bool:
        """Envía datos de telemetría al servidor Fomalhaut"""
        telemetry_type = data.get('type', 'general')
        
        # Determinar endpoint según el tipo
        endpoint_key = telemetry_type if telemetry_type != 'general' else 'telemetry'
        endpoint = self.config['server']['endpoints'].get(endpoint_key, '/api/telemetry')
        url = self.config['server']['base_url'] + endpoint
        
        headers = {
            'Content-Type': 'application/json',
            'User-Agent': 'ESP32-Bridge/1.0'
        }
        
        for attempt in range(self.config['retry_attempts']):
            try:
                response = self.session.post(url, json=data, headers=headers, timeout=5)
                if response.status_code in [200, 201, 202]:
                    self.stats['packets_sent'] += 1
                    if self.config['debug']:
                        print(f"✅ [{telemetry_type}] Enviado → {response.status_code}")
                    return True
                else:
                    print(f"⚠️  Respuesta inesperada: {response.status_code}")
            except requests.exceptions.ConnectionError:
                if attempt == 0:
                    print(f"⚠️  No se pudo conectar al servidor en {url}")
                    print(f"💡 Verifica que el servidor Java-Spring esté ejecutándose")
                time.sleep(1)
            except requests.exceptions.Timeout:
                print(f"⏱️  Timeout en intento {attempt + 1}/{self.config['retry_attempts']}")
            except Exception as e:
                print(f"❌ Error al enviar datos: {e}")
                break
        
        self.stats['errors'] += 1
        return False
    
    def run(self):
        """Loop principal del bridge"""
        print("\n" + "="*60)
        print("🛰️  ESP32 → FOMALHAUT BRIDGE")
        print("="*60)
        print(f"📡 Puerto Serial: {self.config['serial']['port']}")
        print(f"🌐 Servidor: {self.config['server']['base_url']}")
        print(f"🐛 Debug: {'ON' if self.config['debug'] else 'OFF'}")
        print("="*60 + "\n")
        
        if not self.connect_serial():
            return
        
        print("🚀 Bridge iniciado. Leyendo datos del ESP32...")
        print("   (Presiona Ctrl+C para detener)\n")
        
        try:
            while True:
                if self.serial_port.in_waiting > 0:
                    try:
                        line = self.serial_port.readline().decode('utf-8', errors='ignore')
                        self.stats['lines_read'] += 1
                        
                        # Mostrar línea cruda si debug está activado
                        if self.config['debug']:
                            print(f"📥 {line.strip()}")
                        
                        # Parsear y enviar al servidor
                        parsed_data = self.parse_log_line(line)
                        if parsed_data:
                            self.send_to_server(parsed_data)
                    
                    except UnicodeDecodeError:
                        # Ignorar líneas que no se pueden decodificar
                        pass
                
                # Pequeña pausa para no saturar la CPU
                time.sleep(0.01)
        
        except KeyboardInterrupt:
            print("\n\n🛑 Deteniendo bridge...")
            self.print_stats()
        
        finally:
            if self.serial_port and self.serial_port.is_open:
                self.serial_port.close()
                print("✅ Puerto serial cerrado")
    
    def print_stats(self):
        """Imprime estadísticas del bridge"""
        print("\n" + "="*60)
        print("📊 ESTADÍSTICAS DEL BRIDGE")
        print("="*60)
        print(f"📥 Líneas leídas:      {self.stats['lines_read']}")
        print(f"✅ Paquetes enviados:  {self.stats['packets_sent']}")
        print(f"❌ Errores:            {self.stats['errors']}")
        if self.stats['lines_read'] > 0:
            success_rate = (self.stats['packets_sent'] / self.stats['lines_read']) * 100
            print(f"📈 Tasa de éxito:      {success_rate:.1f}%")
        print("="*60 + "\n")


def main():
    """Función principal"""
    config_file = "config.json"
    
    # Permitir especificar archivo de configuración como argumento
    if len(sys.argv) > 1:
        config_file = sys.argv[1]
    
    bridge = ESP32Bridge(config_file)
    bridge.run()


if __name__ == "__main__":
    main()
