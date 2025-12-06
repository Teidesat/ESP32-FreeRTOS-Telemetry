#!/usr/bin/env python3
"""
Simulador de ESP32 para Testing
================================
Simula la salida serial del ESP32 para probar el bridge sin hardware.

Uso:
    python3 simulate_esp32.py | python3 esp32_to_fomalhaut_bridge.py

O en terminales separadas:
    Terminal 1: python3 simulate_esp32.py
    Terminal 2: Leer desde el puerto serial virtual creado
"""

import time
import random
import sys
from datetime import datetime

def generate_system_telemetry():
    """Genera telemetría de sistema aleatoria"""
    cpu = random.randint(10, 95)
    ram = random.randint(128, 512)
    stack = random.randint(1024, 4096)
    
    formats = [
        f"[Sistema] CPU: {cpu}%, RAM: {ram}KB, Stack: {stack}B",
        f"Sistema: CPU={cpu}% RAM libre={ram}KB",
        f"CPU: {cpu}% RAM: {ram}KB"
    ]
    return random.choice(formats)

def generate_power_telemetry():
    """Genera telemetría de potencia aleatoria"""
    voltage = round(random.uniform(3.3, 4.2), 2)
    current = round(random.uniform(0.1, 1.5), 2)
    power = round(voltage * current, 2)
    
    formats = [
        f"[Potencia] Voltaje: {voltage}V, Corriente: {current}A",
        f"Potencia: V={voltage}V I={current}A P={power}W",
        f"Voltaje: {voltage}V Corriente: {current}A"
    ]
    return random.choice(formats)

def generate_temperature_telemetry():
    """Genera telemetría de temperatura aleatoria"""
    temp = round(random.uniform(15.0, 45.0), 1)
    sensor_id = random.choice(["Sensor1", "Sensor2", "CPU", "Batería"])
    
    formats = [
        f"[Temperatura] {sensor_id}: {temp}°C",
        f"Temp {sensor_id}: {temp}C",
        f"Temperatura {sensor_id} = {temp}°C"
    ]
    return random.choice(formats)

def generate_comms_telemetry():
    """Genera telemetría de comunicaciones aleatoria"""
    rssi = random.randint(-90, -50)
    snr = random.randint(0, 15)
    packets_sent = random.randint(100, 9999)
    packets_recv = random.randint(100, 9999)
    
    formats = [
        f"[Comms] RSSI: {rssi}dBm, SNR: {snr}dB",
        f"Comunicación: RSSI={rssi} SNR={snr} Paquetes TX={packets_sent} RX={packets_recv}",
        f"RSSI: {rssi}dBm SNR: {snr}dB"
    ]
    return random.choice(formats)

def generate_general_log():
    """Genera logs generales"""
    messages = [
        "🛰️  Sistema de telemetría iniciado",
        "✅ Sensor de temperatura OK",
        "⚠️  Advertencia: Temperatura elevada",
        "📡 Estableciendo comunicación...",
        "✅ Enlace establecido",
        "🔋 Batería al 85%",
        "🚀 Iniciando secuencia de transmisión",
        "✅ Datos enviados correctamente",
        "🌡️  Calibrando sensores...",
        "💾 Guardando datos en memoria flash",
    ]
    return random.choice(messages)

def print_startup_banner():
    """Imprime banner de inicio simulando el ESP32"""
    print("\n" + "="*60)
    print("🛰️  TEIDESAT SATELLITE TELEMETRY SYSTEM - ESP32 WOKWI")
    print("="*60)
    print("Starting FreeRTOS tasks...")
    print("✅ All telemetry tasks created successfully")
    print("📡 System operational - Telemetry data generation started")
    print("-"*60 + "\n")
    time.sleep(1)

def simulate_esp32():
    """Loop principal de simulación"""
    print_startup_banner()
    
    # Generadores de telemetría
    generators = [
        (generate_system_telemetry, 0.3),      # 30% probabilidad
        (generate_power_telemetry, 0.25),      # 25% probabilidad
        (generate_temperature_telemetry, 0.25), # 25% probabilidad
        (generate_comms_telemetry, 0.15),      # 15% probabilidad
        (generate_general_log, 0.05),          # 5% probabilidad
    ]
    
    iteration = 0
    try:
        while True:
            iteration += 1
            
            # Seleccionar generador basado en probabilidades
            rand = random.random()
            cumulative = 0.0
            
            for generator, probability in generators:
                cumulative += probability
                if rand <= cumulative:
                    line = generator()
                    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                    print(f"[{timestamp}] {line}")
                    sys.stdout.flush()  # Forzar flush para que el bridge lo lea
                    break
            
            # Cada 20 iteraciones, mostrar estadísticas
            if iteration % 20 == 0:
                print(f"\n--- Estadísticas (Iteración {iteration}) ---")
                print(f"Tiempo de ejecución: {iteration * 2} segundos")
                print(f"Líneas generadas: {iteration}")
                print("-"*60 + "\n")
            
            # Intervalo entre mensajes (2 segundos)
            time.sleep(2)
    
    except KeyboardInterrupt:
        print("\n\n🛑 Simulación detenida por el usuario")
        print(f"📊 Total de líneas generadas: {iteration}")
        sys.exit(0)

def main():
    """Función principal"""
    print("="*60)
    print("🧪 SIMULADOR DE ESP32 - TESTING MODE")
    print("="*60)
    print("Este script simula la salida serial del ESP32")
    print("Presiona Ctrl+C para detener")
    print("="*60 + "\n")
    
    time.sleep(1)
    simulate_esp32()

if __name__ == "__main__":
    main()
