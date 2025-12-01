# 🔌 Especificación API para Backend de Fomalhaut

## 📋 Documento de Coordinación

**Para:** Equipo Backend Java-Spring  
**De:** Equipo ESP32 / TeideSat  
**Fecha:** Diciembre 2025  
**Asunto:** Especificación de endpoints para recepción de telemetría

---

## 🎯 Objetivo

El ESP32 generará datos de telemetría que serán enviados al backend a través de un bridge Python. Este documento especifica el formato de datos y endpoints necesarios.

## 📡 Flujo de Datos

```
ESP32 → [Serial] → Bridge Python → [HTTP POST JSON] → Backend Java-Spring
```

## 🔗 Endpoints Requeridos

### Base URL
```
http://localhost:8080/api/telemetry
```

### 1️⃣ Telemetría General
```http
POST /api/telemetry
Content-Type: application/json
```

**Request Body:**
```json
{
  "timestamp": "2025-12-01T10:30:45.123456",
  "type": "general",
  "raw_line": "Sistema iniciado correctamente"
}
```

**Response:**
```json
{
  "status": "received",
  "message": "Telemetry data stored successfully"
}
```

### 2️⃣ Telemetría de Sistema
```http
POST /api/telemetry/system
Content-Type: application/json
```

**Request Body:**
```json
{
  "timestamp": "2025-12-01T10:30:45.123456",
  "type": "system",
  "cpu_usage": 45,
  "ram_free": 234,
  "raw_line": "[Sistema] CPU: 45%, RAM: 234KB"
}
```

**Campos opcionales:**
- `stack_free`: int (bytes libres en el stack)
- `uptime_ms`: long (tiempo de ejecución)
- `task_count`: int (número de tareas activas)

### 3️⃣ Telemetría de Potencia
```http
POST /api/telemetry/power
Content-Type: application/json
```

**Request Body:**
```json
{
  "timestamp": "2025-12-01T10:30:46.789012",
  "type": "power",
  "voltage": 3.7,
  "current": 0.5,
  "raw_line": "[Potencia] Voltaje: 3.7V, Corriente: 0.5A"
}
```

**Campos opcionales:**
- `power`: float (potencia en watts, calculada como V × A)
- `battery_percentage`: int (0-100)

### 4️⃣ Telemetría de Temperatura
```http
POST /api/telemetry/temperature
Content-Type: application/json
```

**Request Body:**
```json
{
  "timestamp": "2025-12-01T10:30:47.345678",
  "type": "temperature",
  "temperature": 25.3,
  "raw_line": "[Temperatura] Sensor1: 25.3°C"
}
```

**Campos opcionales:**
- `sensor_id`: string (identificador del sensor)
- `location`: string (ubicación del sensor)

### 5️⃣ Telemetría de Comunicaciones
```http
POST /api/telemetry/comms
Content-Type: application/json
```

**Request Body:**
```json
{
  "timestamp": "2025-12-01T10:30:48.901234",
  "type": "comms",
  "rssi": -65,
  "snr": 8,
  "raw_line": "[Comms] RSSI: -65dBm, SNR: 8dB"
}
```

**Campos opcionales:**
- `packets_sent`: int
- `packets_received`: int
- `packet_loss`: float (porcentaje)
- `frequency`: float (MHz)

## 🔒 Autenticación (Opcional)

Si el backend requiere autenticación:

```http
POST /api/auth/login
Content-Type: application/json

{
  "username": "esp32_bridge",
  "password": "secure_password"
}
```

**Response:**
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
}
```

Luego incluir en headers:
```http
Authorization: Bearer {token}
```

## 📊 Respuestas HTTP Esperadas

| Código | Significado | Acción del Bridge |
|--------|-------------|-------------------|
| 200 | OK | Continuar enviando |
| 201 | Created | Continuar enviando |
| 400 | Bad Request | Log error, continuar |
| 401 | Unauthorized | Reautenticar |
| 500 | Server Error | Reintentar 3 veces |
| 503 | Unavailable | Esperar 5s, reintentar |

## 🧪 Testing con curl

### Test Endpoint de Sistema
```bash
curl -X POST http://localhost:8080/api/telemetry/system \
  -H "Content-Type: application/json" \
  -d '{
    "timestamp": "2025-12-01T10:30:45.123456",
    "type": "system",
    "cpu_usage": 45,
    "ram_free": 234,
    "raw_line": "[Sistema] CPU: 45%, RAM: 234KB"
  }'
```

### Test Endpoint de Potencia
```bash
curl -X POST http://localhost:8080/api/telemetry/power \
  -H "Content-Type: application/json" \
  -d '{
    "timestamp": "2025-12-01T10:30:46.789012",
    "type": "power",
    "voltage": 3.7,
    "current": 0.5,
    "raw_line": "[Potencia] Voltaje: 3.7V, Corriente: 0.5A"
  }'
```

## 💾 Modelo de Datos Sugerido (JPA/Hibernate)

```java
@Entity
@Table(name = "telemetry_data")
public class TelemetryData {
    
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    
    @Column(nullable = false)
    private String timestamp;
    
    @Column(nullable = false)
    private String type; // system, power, temperature, comms, general
    
    @Column(columnDefinition = "TEXT")
    private String rawLine;
    
    // Campos específicos (nullable)
    private Integer cpuUsage;
    private Integer ramFree;
    private Float voltage;
    private Float current;
    private Float temperature;
    private Integer rssi;
    private Integer snr;
    
    // Getters, setters, constructores...
}
```

## 🎨 Ejemplo de Controller

```java
@RestController
@RequestMapping("/api/telemetry")
@CrossOrigin(origins = "*") // Ajustar según necesidad
public class TelemetryController {
    
    @Autowired
    private TelemetryService telemetryService;
    
    @PostMapping("/system")
    public ResponseEntity<Map<String, String>> receiveSystemTelemetry(
            @RequestBody TelemetryData data) {
        
        // Validar datos
        if (data.getCpuUsage() == null || data.getRamFree() == null) {
            return ResponseEntity.badRequest()
                .body(Map.of("error", "Missing required fields"));
        }
        
        // Guardar en BD
        telemetryService.save(data);
        
        // Emitir a WebSocket (opcional, para frontend en tiempo real)
        messagingTemplate.convertAndSend("/topic/telemetry/system", data);
        
        return ResponseEntity.ok(Map.of(
            "status", "received",
            "message", "System telemetry stored"
        ));
    }
    
    @PostMapping("/power")
    public ResponseEntity<Map<String, String>> receivePowerTelemetry(
            @RequestBody TelemetryData data) {
        // Similar implementación...
        telemetryService.save(data);
        return ResponseEntity.ok(Map.of("status", "received"));
    }
    
    // Más endpoints...
}
```

## 🔄 WebSocket para Tiempo Real (Opcional)

Si el frontend necesita datos en tiempo real:

```java
@Configuration
@EnableWebSocketMessageBroker
public class WebSocketConfig implements WebSocketMessageBrokerConfigurer {
    
    @Override
    public void configureMessageBroker(MessageBrokerRegistry config) {
        config.enableSimpleBroker("/topic");
        config.setApplicationDestinationPrefixes("/app");
    }
    
    @Override
    public void registerStompEndpoints(StompEndpointRegistry registry) {
        registry.addEndpoint("/ws").setAllowedOrigins("*").withSockJS();
    }
}
```

Frontend puede suscribirse:
```javascript
const socket = new SockJS('http://localhost:8080/ws');
const stompClient = Stomp.over(socket);

stompClient.connect({}, () => {
    stompClient.subscribe('/topic/telemetry/system', (message) => {
        const data = JSON.parse(message.body);
        // Actualizar UI
    });
});
```

## 📈 Tasa de Datos Esperada

- **Frecuencia:** 1-10 mensajes/segundo (configurable)
- **Volumen:** ~500 bytes/mensaje
- **Total:** ~5-50 KB/s
- **Concurrencia:** 1 ESP32 (escalable a múltiples dispositivos)

## ✅ Checklist de Implementación Backend

- [ ] Crear controladores REST para cada endpoint
- [ ] Crear modelo de datos (JPA Entity)
- [ ] Implementar servicio de persistencia
- [ ] Configurar CORS para permitir requests del bridge
- [ ] (Opcional) Implementar WebSocket para tiempo real
- [ ] (Opcional) Implementar autenticación JWT
- [ ] Añadir validación de datos de entrada
- [ ] Implementar logging de errores
- [ ] Crear tests unitarios para endpoints
- [ ] Documentar API con Swagger/OpenAPI