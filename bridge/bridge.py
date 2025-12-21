#!/usr/bin/env python3
"""
Bridge ESP32 → Fomalhaut v2
===========================

Propósito
---------
Puente en Python que lee líneas JSON desde el puerto serie del ESP32 y las
reenvía al backend local (Java/Spring) de Fomalhaut. Filtra únicamente
mensajes con formato JSON y enruta según el campo `type`.

Contrato de datos de entrada (desde ESP32)
------------------------------------------
- Una línea por mensaje (terminada en `\n`).
- Debe empezar por `{` (JSON válido).
- Campo `type` esperado: `system`, `power`, `temperature`, `comms`.
    Se ignoran otros tipos o líneas no JSON.

Salida (hacia backend)
----------------------
- Peticiones HTTP POST con cuerpo JSON.
- Endpoint por tipo definido en `config.json` → `server.endpoints`.
- Se añade `timestamp` (ISO-8601) del bridge para trazabilidad.

Errores y manejo
----------------
- Líneas no JSON: se ignoran y se contabilizan en `non_json_ignored`.
- JSON inválido: el parsing falla silenciosamente y se descarta.
- Conexión HTTP: se informa del error y no interrumpe el bucle principal.

Requisitos
----------
- Python 3.8+ y paquetes `pyserial`, `requests` (ver `requirements.txt`).
- Backend en `config.json['server']['base_url']` disponible.
- Permisos para acceder al puerto serie (Linux: grupo `dialout`).
"""

import serial
import requests
import json
import time
from datetime import datetime
from typing import Dict, Optional, Any
import sys

class ESP32Bridge:
    def __init__(self, config_path: str = "config.json"):
        """Inicializa el bridge con la configuración especificada.

        Args:
            config_path: Ruta al archivo JSON de configuración.

        Atributos:
            config: Diccionario con la configuración cargada.
            serial_port: Handler del puerto serie (`serial.Serial`) o `None`.
            session: Sesión HTTP reutilizable (`requests.Session`).
            stats: Contadores de actividad del bridge.
        """
        self.config = self.load_config(config_path)
        self.serial_port = None
        self.session = requests.Session()
        # Contadores básicos para telemetría del propio bridge.
        # - lines_read: total de líneas leídas del puerto serie
        # - packets_sent: envíos HTTP aceptados (200/201/202)
        # - errors: errores durante envío/parsing (se recomienda incrementar
        #   en cada excepción relevante)
        # - json_parsed: JSON válidos parseados
        # - non_json_ignored: líneas descartadas por no ser JSON
        self.stats = {
            'lines_read': 0,
            'packets_sent': 0,
            'errors': 0,
            'json_parsed': 0,
            'non_json_ignored': 0
        }
        
    def load_config(self, config_path: str) -> Dict[str, Any]:
        """Carga la configuración desde un archivo JSON.

        Args:
            config_path: Ruta al fichero de configuración.

        Returns:
            Diccionario de configuración.

        Nota:
            Si el archivo no existe, se imprime un error y retorna `None`.
        """
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            print(f"❌ Archivo {config_path} no encontrado")
            return None
    
    def connect_serial(self) -> bool:
        """Conecta al puerto serie del ESP32 según `config`.

        Returns:
            `True` si la conexión fue exitosa, `False` en caso contrario.

        Manejo:
            - Resetea el buffer de entrada para comenzar limpio.
            - Informa la configuración usada (puerto y baudrate).
        """
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
    
    def parse_log_line(self, line: str) -> Optional[Dict[str, Any]]:
        """Parsea una línea proveniente del ESP32.

        Args:
            line: Línea cruda leída del puerto serie.

        Returns:
            Diccionario con el JSON parseado y `timestamp` del bridge, o `None`
            si la línea no es JSON válido o su `type` no es admitido.

        Detalles:
            - Ignora líneas vacías o que no empiezan por `{`.
            - Tipos válidos (por defecto): `system`, `power`, `temperature`, `comms`.
            - Incrementa contador `json_parsed` tras parseo exitoso.
        """
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
            # JSON inválido: se descarta sin interrumpir el bucle principal.
            # Recomendación: incrementar self.stats['errors'] si se desea medir.
            return None
    
    def send_to_server(self, data: Dict[str, Any]) -> bool:
        """Envía datos al servidor backend según su tipo.

        Args:
            data: JSON preparado para envío (incluye `type`).

        Returns:
            `True` si el servidor respondió 200/201/202; `False` en otros casos o
            ante excepciones de conexión.

        Notas:
            - Endpoint se selecciona desde `config['server']['endpoints']` por tipo;
              si no existe, se usa `/api/telemetry` por defecto.
            - Se usa `requests.Session` para eficiencia.
        """
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
                # Mostrar código real devuelto por el backend para mayor claridad.
                print(f"✅ [{telemetry_type.upper():7}] → {response.status_code}")
                return True
            else:
                print(f"⚠️  [{telemetry_type}] Status: {response.status_code}")
                return False
        except requests.exceptions.ConnectionError:
            print(f"❌ No hay conexión con {url}")
            return False
        except Exception as e:
            print(f"❌ Error: {e}")
            # Recomendación: incrementar self.stats['errors'] para visibilidad.
            return False
    
    def run(self):
        """Loop principal del bridge.

        - Conecta al puerto serie.
        - Lee líneas de forma no bloqueante (`in_waiting`).
        - Parsea y envía JSON válidos al backend.
        - Control de flujo ligero con `time.sleep(0.01)`.
        """
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
                # Lectura no bloqueante: solo si hay datos disponibles.
                if self.serial_port.in_waiting > 0:
                    try:
                        line = self.serial_port.readline().decode('utf-8', errors='ignore')
                        self.stats['lines_read'] += 1
                        
                        parsed = self.parse_log_line(line)
                        if parsed:
                            self.send_to_server(parsed)
                    
                    except UnicodeDecodeError:
                        # Si aparece una línea con codificación inválida, se ignora.
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
        """Imprime estadísticas acumuladas del bridge.

        Muestra totales de líneas leídas, JSON parseados, non-JSON ignorados,
        paquetes enviados y contadores de errores.
        """
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
    """Punto de entrada principal.

    Carga la configuración por defecto y ejecuta el loop del bridge si la
    configuración existe; de lo contrario, informa del problema.
    """
    bridge = ESP32Bridge("config.json")
    if bridge.config:
        bridge.run()
    else:
        print("❌ No se pudo cargar la configuración")


if __name__ == "__main__":
    main()
