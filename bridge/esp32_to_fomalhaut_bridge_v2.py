#!/usr/bin/env python3
"""
ESP32 to Fomalhaut Bridge v2
=========================
Script puente mejorado que lee logs del ESP32 por puerto serial y los envía al servidor
local Java-Spring de Fomalhaut.

Cambios en v2:
- Mejor manejo de JSON
- Más información de debug
- Mejor detección de errores de parsing
"""

import serial
import requests
import json
import time
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
            'errors': 0,
            'json_parsed': 0,
            'non_json_ignored': 0
        }
        
    def load_config(self, config_path: str) -> Dict:
        """Carga la configuración desde un archivo JSON"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            print(f"❌ Archivo {config_path} no encontrado")
            return None
    
    def connect_serial(self) -> bool:
        """Conecta al puerto serial del ESP32"""
        try:
            self.serial_port = serial.Serial(
                port=self.config['serial']['port'],
                baudrate=self.config['serial']['baudrate'],
                timeout=self.config['serial']['timeout']
            )
            # Limpiar buffer
            self.serial_port.reset_input_buffer()
            print(f"✅ Conectado a {self.config['serial']['port']} @ {self.config['serial']['baudrate']} baud")
            return True
        except serial.SerialException as e:
            print(f"❌ Error: {e}")
            return False
    
    def parse_log_line(self, line: str) -> Optional[Dict]:
        """Parsea una línea de log del ESP32"""
        line = line.strip()
        if not line:
            return None
        
        # Solo procesar JSON
        if not line.startswith('{'):
            self.stats['non_json_ignored'] += 1
            return None
        
        try:
            json_data = json.loads(line)
            telemetry_type = json_data.get('type', 'general')
            
            # Filtrar tipos válidos
            valid_types = ['system', 'power', 'temperature', 'comms']
            if telemetry_type not in valid_types:
                return None
            
            # Agregar timestamp del bridge
            json_data['timestamp'] = datetime.now().isoformat()
            self.stats['json_parsed'] += 1
            return json_data
        
        except json.JSONDecodeError as e:
            return None
    
    def send_to_server(self, data: Dict) -> bool:
        """Envía datos al servidor"""
        telemetry_type = data.get('type', 'general')
        endpoint = self.config['server']['endpoints'].get(telemetry_type, '/api/telemetry')
        url = self.config['server']['base_url'] + endpoint
        
        try:
            response = self.session.post(
                url, 
                json=data, 
                headers={'Content-Type': 'application/json'},
                timeout=5
            )
            if response.status_code in [200, 201, 202]:
                self.stats['packets_sent'] += 1
                print(f"✅ [{telemetry_type.upper():7}] → 201")
                return True
            else:
                print(f"⚠️  [{telemetry_type}] Status: {response.status_code}")
                return False
        except requests.exceptions.ConnectionError:
            print(f"❌ No hay conexión con {url}")
            return False
        except Exception as e:
            print(f"❌ Error: {e}")
            return False
    
    def run(self):
        """Loop principal"""
        print("\n" + "="*60)
        print("🛰️  ESP32 → FOMALHAUT BRIDGE v2")
        print("="*60)
        print(f"📡 Puerto: {self.config['serial']['port']}")
        print(f"🌐 Server: {self.config['server']['base_url']}")
        print("="*60 + "\n")
        
        if not self.connect_serial():
            return
        
        print("🚀 Escuchando datos del ESP32...")
        print("   (Presiona Ctrl+C para detener)\n")
        
        try:
            while True:
                if self.serial_port.in_waiting > 0:
                    try:
                        line = self.serial_port.readline().decode('utf-8', errors='ignore')
                        self.stats['lines_read'] += 1
                        
                        parsed = self.parse_log_line(line)
                        if parsed:
                            self.send_to_server(parsed)
                    
                    except UnicodeDecodeError:
                        pass
                
                time.sleep(0.01)
        
        except KeyboardInterrupt:
            print("\n\n🛑 Deteniendo...")
            self.print_stats()
        
        finally:
            if self.serial_port and self.serial_port.is_open:
                self.serial_port.close()
                print("✅ Puerto serial cerrado")
    
    def print_stats(self):
        """Imprime estadísticas"""
        print("\n" + "="*60)
        print("📊 ESTADÍSTICAS")
        print("="*60)
        print(f"📥 Líneas leídas:        {self.stats['lines_read']}")
        print(f"📄 JSON parseados:       {self.stats['json_parsed']}")
        print(f"⏭️  Non-JSON ignorados:    {self.stats['non_json_ignored']}")
        print(f"✅ Paquetes enviados:    {self.stats['packets_sent']}")
        print(f"❌ Errores:              {self.stats['errors']}")
        print("="*60 + "\n")


def main():
    bridge = ESP32Bridge("config.json")
    if bridge.config:
        bridge.run()
    else:
        print("❌ No se pudo cargar la configuración")


if __name__ == "__main__":
    main()
