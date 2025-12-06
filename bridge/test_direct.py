import json
import time
import requests

# Simular algunos logs
logs = [
    {"timestamp": "2025-12-04T23:50:00.000000", "type": "system", "cpu_usage": 45, "ram_free": 234, "raw_line": "[Sistema] CPU: 45%, RAM: 234KB"},
    {"timestamp": "2025-12-04T23:50:01.000000", "type": "power", "voltage": 3.7, "current": 0.5, "raw_line": "[Potencia] Voltaje: 3.7V, Corriente: 0.5A"},
    {"timestamp": "2025-12-04T23:50:02.000000", "type": "temperature", "temperature": 25.3, "raw_line": "[Temperatura] Sensor1: 25.3°C"},
    {"timestamp": "2025-12-04T23:50:03.000000", "type": "comms", "rssi": -65, "snr": 8, "raw_line": "[Comms] RSSI: -65dBm, SNR: 8dB"},
]

print("Enviando datos de prueba al backend...")
for log in logs:
    try:
        response = requests.post(f'http://localhost:20001/api/telemetry/{log["type"]}', json=log, timeout=5)
        print(f"✅ {log['type']}: {response.status_code}")
    except Exception as e:
        print(f"❌ Error: {e}")
    time.sleep(0.5)

# Verificar datos
print("\nVerificando datos en el backend...")
time.sleep(1)
try:
    response = requests.get('http://localhost:20001/api/telemetry', timeout=5)
    data = response.json()
    print(f"✅ Total logs: {len(data)}")
    if data:
        print(f"📊 Primer log:\n{json.dumps(data[0], indent=2)}")
except Exception as e:
    print(f"❌ Error: {e}")
