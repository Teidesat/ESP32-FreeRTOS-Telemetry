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
                    "base_url": "http://localhost:20001",
                    "endpoints": {
                        "telemetry": "/api/telemetry",
                        "system": "/api/telemetry/system",
                        "power": "/api/telemetry/power",
                        "temperature": "/api/telemetry/temperature",
                        "comms": "/api/telemetry/comms"
                    }
                },
                "debug": False,
                "batch_size": 10,
                "retry_attempts": 3,
                "retry_delay": 1
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
        Parsea una línea de log del ESP32.
        
        Ahora el ESP32 envía datos en formato JSON:
        {"type":"system","cpuUsage":45,"memoryFree":234567,...}
        {"type":"power","voltage":3.7,"current":0.5,...}
        {"type":"temperature","obcTemp":25.3,...}
        {"type":"comms","rssi":-65,"snr":8,...}
        
        También ignora logs de texto que no sean JSON.
        """
        line = line.strip()
        if not line or line.startswith('---') or line.startswith('==='):
            return None
        
        # Solo procesar líneas que comienzan con { (JSON)
        if not line.startswith('{'):
            # Ignorar logs de texto que no sean JSON
            return None
        
        try:
            # Parsear JSON directamente
            json_data = json.loads(line)
            telemetry_type = json_data.get('type', 'general')
            
            # No enviar logs de tipo "general" que no sean de nuestros tipos específicos
            if telemetry_type == 'general':
                return None
            
            # Validar que sea uno de los tipos conocidos
            valid_types = ['system', 'power', 'temperature', 'comms']
            if telemetry_type not in valid_types:
                return None
            
            # Agregar timestamp
            json_data['timestamp'] = datetime.now().isoformat()
            return json_data
        
        except json.JSONDecodeError:
            # No es JSON válido, ignorar
            return None
    
    def send_to_server(self, data: Dict) -> bool:
        """Envía datos de telemetría al servidor Fomalhaut"""
        telemetry_type = data.get('type', 'general')
        
        # Ignorar logs de tipo "general"
        if telemetry_type == 'general':
            return False
        
        # Determinar endpoint según el tipo
        endpoint = self.config['server']['endpoints'].get(telemetry_type, '/api/telemetry')
        url = self.config['server']['base_url'] + endpoint
        
        headers = {
            'Content-Type': 'application/json',
            'User-Agent': 'ESP32-Bridge/1.0'
        }
        
        for attempt in range(self.config.get('retry_attempts', 3)):
            try:
                response = self.session.post(url, json=data, headers=headers, timeout=5)
                if response.status_code in [200, 201, 202]:
                    self.stats['packets_sent'] += 1
                    if self.config.get('debug', False):
                        print(f"✅ [{telemetry_type.upper()}] Enviado → {response.status_code}")
                    return True
                else:
                    print(f"⚠️  [{telemetry_type}] Respuesta inesperada: {response.status_code}")
            except requests.exceptions.ConnectionError:
                if attempt == 0:
                    print(f"⚠️  No se pudo conectar al servidor en {url}")
                    print(f"💡 Verifica que el servidor Java-Spring esté ejecutándose")
                time.sleep(1)
            except requests.exceptions.Timeout:
                if self.config.get('debug', False):
                    print(f"⏱️  Timeout en intento {attempt + 1}/{self.config.get('retry_attempts', 3)}")
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
        print(f"🐛 Debug: {'ON' if self.config.get('debug', False) else 'OFF'}")
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
                        if self.config.get('debug', False):
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
