#!/usr/bin/env python3
"""
Script de testing para el ESP32 → Fomalhaut Bridge
===================================================
Simula logs del ESP32 y verifica que el parsing funcione correctamente.
"""

import sys
import os

# Añadir el directorio actual al path para importar el bridge
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from esp32_to_fomalhaut_bridge import ESP32Bridge

def test_parse_system_log():
    """Test de parsing de logs de sistema"""
    bridge = ESP32Bridge()
    
    test_cases = [
        "[Sistema] CPU: 45%, RAM: 234KB",
        "CPU: 78% RAM: 1024KB Stack: 2048",
        "Sistema: CPU 23%, RAM libre: 512KB",
    ]
    
    print("🧪 Testing System Log Parsing...")
    for line in test_cases:
        result = bridge.parse_log_line(line)
        if result:
            print(f"  ✅ '{line}'")
            print(f"     → type={result['type']}, cpu={result.get('cpu_usage')}, ram={result.get('ram_free')}")
        else:
            print(f"  ⚠️  '{line}' → No parsed")
    print()

def test_parse_power_log():
    """Test de parsing de logs de potencia"""
    bridge = ESP32Bridge()
    
    test_cases = [
        "[Potencia] Voltaje: 3.7V, Corriente: 0.5A",
        "Voltage: 5.0V Current: 1.2A",
        "Potencia: Voltaje=3.3V Corriente=0.8A",
    ]
    
    print("🧪 Testing Power Log Parsing...")
    for line in test_cases:
        result = bridge.parse_log_line(line)
        if result:
            print(f"  ✅ '{line}'")
            print(f"     → type={result['type']}, voltage={result.get('voltage')}, current={result.get('current')}")
        else:
            print(f"  ⚠️  '{line}' → No parsed")
    print()

def test_parse_temperature_log():
    """Test de parsing de logs de temperatura"""
    bridge = ESP32Bridge()
    
    test_cases = [
        "[Temperatura] Sensor1: 25.3°C",
        "Temp: 42.5°C",
        "Temperatura ambiente: 18.7 C",
    ]
    
    print("🧪 Testing Temperature Log Parsing...")
    for line in test_cases:
        result = bridge.parse_log_line(line)
        if result:
            print(f"  ✅ '{line}'")
            print(f"     → type={result['type']}, temp={result.get('temperature')}")
        else:
            print(f"  ⚠️  '{line}' → No parsed")
    print()

def test_parse_comms_log():
    """Test de parsing de logs de comunicaciones"""
    bridge = ESP32Bridge()
    
    test_cases = [
        "[Comms] RSSI: -65dBm, SNR: 8dB",
        "RSSI: -72dBm SNR: 10dB Paquetes: 1234",
        "Comunicación: RSSI=-80 SNR=5",
    ]
    
    print("🧪 Testing Comms Log Parsing...")
    for line in test_cases:
        result = bridge.parse_log_line(line)
        if result:
            print(f"  ✅ '{line}'")
            print(f"     → type={result['type']}, rssi={result.get('rssi')}, snr={result.get('snr')}")
        else:
            print(f"  ⚠️  '{line}' → No parsed")
    print()

def test_json_generation():
    """Test de generación de JSON completo"""
    bridge = ESP32Bridge()
    
    test_line = "[Sistema] CPU: 45%, RAM: 234KB"
    result = bridge.parse_log_line(test_line)
    
    print("🧪 Testing JSON Generation...")
    if result:
        import json
        print(f"  Input: {test_line}")
        print(f"  JSON Output:")
        print(json.dumps(result, indent=4))
    else:
        print("  ❌ Failed to parse line")
    print()

def main():
    """Ejecuta todos los tests"""
    print("\n" + "="*60)
    print("🧪 ESP32 → FOMALHAUT BRIDGE - TEST SUITE")
    print("="*60 + "\n")
    
    test_parse_system_log()
    test_parse_power_log()
    test_parse_temperature_log()
    test_parse_comms_log()
    test_json_generation()
    
    print("="*60)
    print("✅ Tests completados")
    print("="*60 + "\n")
    
    print("💡 Para probar con datos reales del ESP32:")
    print("   1. Conecta el ESP32")
    print("   2. Ejecuta: python3 esp32_to_fomalhaut_bridge.py")
    print()

if __name__ == "__main__":
    main()
