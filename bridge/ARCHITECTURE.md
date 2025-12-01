# 📐 Arquitectura ESP32 → Fomalhaut

## Diagrama de Flujo de Datos

```
┌─────────────────────────────────────────────────────────────────┐
│                         ESP32 - TeideSat                         │
│                                                                  │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐       │
│  │   Sensores   │──▶│  Telemetría  │──▶│    Logger    │       │
│  │  (Hardware)  │   │  Processing  │   │  (LittleFS)  │       │
│  └──────────────┘   └──────────────┘   └──────┬───────┘       │
│                                                 │                │
│                                                 ▼                │
│                                      ┌──────────────────┐       │
│                                      │  Puerto Serial   │       │
│                                      │  (115200 baud)   │       │
│                                      └────────┬─────────┘       │
└──────────────────────────────────────────────┼──────────────────┘
                                                │
                                                │ USB/UART
                                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Ordenador Local (Linux)                     │
│                                                                  │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │         🌉 ESP32 → Fomalhaut Bridge (Python)              │ │
│  │                                                            │ │
│  │  ┌─────────────┐   ┌────────────┐   ┌────────────────┐  │ │
│  │  │   Serial    │──▶│   Parser   │──▶│  HTTP Client   │  │ │
│  │  │   Reader    │   │   (Regex)  │   │  (Requests)    │  │ │
│  │  └─────────────┘   └────────────┘   └────────┬───────┘  │ │
│  └──────────────────────────────────────────────┼──────────┘ │
│                                                  │             │
│                                                  │ HTTP POST   │
│                                                  │ (JSON)      │
│                                                  ▼             │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │       Servidor Local Java-Spring (Backend)                │ │
│  │                                                            │ │
│  │  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐ │ │
│  │  │  REST API    │──▶│   Business   │──▶│   Database   │ │ │
│  │  │  Controller  │   │    Logic     │   │  (Opcional)  │ │ │
│  │  └──────┬───────┘   └──────────────┘   └──────────────┘ │ │
│  └─────────┼────────────────────────────────────────────────┘ │
│            │                                                   │
│            │ WebSocket / HTTP                                  │
│            ▼                                                   │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │           Frontend Fomalhaut (React + Vite)               │ │
│  │                                                            │ │
│  │  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐ │ │
│  │  │   Dashboard  │   │    Gráficas  │   │   Alertas    │ │ │
│  │  │  Telemetría  │   │   en Tiempo  │   │     Real     │ │ │
│  │  └──────────────┘   │     Real     │   └──────────────┘ │ │
│  │                     └──────────────┘                     │ │
│  └────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

## 🔄 Flujo de Datos Detallado

### 1️⃣ Generación en ESP32
```
Sensor → Procesamiento → Logger → Serial
   ↓
Formato: "[Sistema] CPU: 45%, RAM: 234KB"
```

### 2️⃣ Captura por Bridge Python
```python
line = serial_port.readline()
# → "[Sistema] CPU: 45%, RAM: 234KB"

parsed_data = parse_log_line(line)
# → {
#     "type": "system",
#     "cpu_usage": 45,
#     "ram_free": 234,
#     "timestamp": "2025-12-01T10:30:45.123456"
#   }
```

### 3️⃣ Envío al Servidor
```python
requests.post(
    "http://localhost:8080/api/telemetry/system",
    json=parsed_data
)
# → HTTP 200 OK
```

### 4️⃣ Procesamiento en Backend
```java
@PostMapping("/api/telemetry/system")
public ResponseEntity<String> receiveSystemTelemetry(@RequestBody TelemetryData data) {
    // Validar datos
    // Guardar en DB (opcional)
    // Emitir WebSocket a frontend
    return ResponseEntity.ok("Received");
}
```

### 5️⃣ Visualización en Frontend
```javascript
// React Component
useEffect(() => {
    // Recibir datos vía WebSocket o polling
    fetchTelemetryData();
}, []);

// Mostrar en dashboard con gráficas en tiempo real
```

## 📊 Tipos de Telemetría Soportados

| Tipo | Origen (ESP32) | Bridge | Backend Endpoint |
|------|----------------|--------|------------------|
| **Sistema** | CPU, RAM, Stack | ✅ | `/api/telemetry/system` |
| **Potencia** | Voltaje, Corriente | ✅ | `/api/telemetry/power` |
| **Temperatura** | Sensores térmicos | ✅ | `/api/telemetry/temperature` |
| **Comunicaciones** | RSSI, SNR, Paquetes | ✅ | `/api/telemetry/comms` |
| **General** | Eventos, Logs | ✅ | `/api/telemetry` |

## 🔧 Configuración por Componente

### ESP32 (`platformio.ini`)
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

### Bridge (`config.json`)
```json
{
  "serial": {
    "port": "/dev/ttyUSB0",
    "baudrate": 115200
  },
  "server": {
    "base_url": "http://localhost:8080"
  }
}
```

### Backend (`application.properties`)
```properties
server.port=8080
spring.application.name=Fomalhaut-Backend
```

### Frontend (`vite.config.ts`)
```typescript
export default defineConfig({
  server: {
    port: 20001,
    proxy: {
      '/api': 'http://localhost:8080'
    }
  }
})
```

## ⚡ Rendimiento Estimado

- **Latencia Serial → Bridge**: < 10ms
- **Latencia Bridge → Backend**: < 50ms (red local)
- **Throughput**: Hasta 500 líneas/segundo
- **Uso CPU Bridge**: < 5%
- **Uso RAM Bridge**: ~20MB

## 🛡️ Manejo de Errores

```
ESP32 desconectado → Bridge reintenta conexión cada 5s
Backend caído → Bridge almacena en cola (futuro: implementar)
Parse fallido → Envía raw_line al backend
Timeout HTTP → 3 reintentos automáticos
```

## 🚀 Orden de Inicio Recomendado

```bash
# Terminal 1: Iniciar Backend
cd /path/to/backend
./mvnw spring-boot:run

# Terminal 2: Iniciar Frontend
cd Fomalhaut
npm run dev

# Terminal 3: Subir código al ESP32
cd /home/u/Teidesat/probando
platformio run --target upload

# Terminal 4: Iniciar Bridge
cd bridge
python3 esp32_to_fomalhaut_bridge.py
```