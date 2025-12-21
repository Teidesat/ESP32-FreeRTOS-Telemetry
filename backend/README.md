## Fomalhaut Backend (Java/Spring)

Servidor REST que recibe telemetrías del ESP32 a través del bridge Python y las almacena en una base de datos H2 en memoria. Expone endpoints para insertar, consultar y limpiar telemetrías.

### Arquitectura rápida

- Framework: Spring Boot 3.1 (Java 17)
- Persistencia: Spring Data JPA + H2 (in-memory)
- Contexto REST: `server.servlet.context-path=/api` → los endpoints comienzan en `/api/...`
- Puerto: `20001`
- CORS: Abierto a `*` (ver bean en `FomalhautBackendApplication`).

Estructura principal:
- `FomalhautBackendApplication.java`: punto de entrada y configuración CORS.
- `controller/TelemetryController.java`: endpoints REST (`/api/telemetry/...`).
- `model/Telemetry.java`: entidad JPA que representa una línea de telemetría.
- `repository/TelemetryRepository.java`: consultas JPA y nativas.
- `resources/application.properties`: configuración de servidor, JPA y H2.

### Endpoints

Base: `http://localhost:20001/api/telemetry`

- `GET /api/telemetry` → lista todas las telemetrías (orden descendente por `createdAt`).
- `GET /api/telemetry/system|power|temperature|comms` → lista por tipo.
- `GET /api/telemetry/latest/{count}` → devuelve las últimas `count` entradas.
- `POST /api/telemetry` → inserta telemetría general.
- `POST /api/telemetry/system|power|temperature|comms` → inserta telemetría específica por tipo.
- `DELETE /api/telemetry/clear` → elimina todas las telemetrías.
- `DELETE /api/telemetry/{id}` → elimina una telemetría por ID.

Respuestas típicas:
- Inserción: `201 Created` + cuerpo `{status, message, id, type}`.
- Consulta: `200 OK` + lista de objetos `Telemetry`.

### Modelo de datos (`Telemetry`)

Campos principales:
- `id`: identificador autogenerado.
- `timestamp`: recibido del bridge/ESP32 (ISO-8601 o equivalente).
- `type`: `system`, `power`, `temperature`, `comms`, `general`.
- `rawLine`: línea original (opcional), útil para auditoría.
- Específicos por tipo:
	- System: `cpuUsage`, `memoryFree`, `uptime`, `taskCount`, `cpuTemp`.
	- Power: `voltage`, `current`, `solarVoltage`, `solarCurrent`, `batteryLevel`, `batteryTemp`.
	- Temperature: `obcTemp`, `commsTemp`, `payloadTemp`, `batteryTempFloat`, `externalTemp`.
	- Comms: `rssi`, `snr`, `commsUptime`, `successRate`.
- `createdAt`: se establece automáticamente en `@PrePersist`.

Nota: no es obligatorio llenar todos los campos; JSONs parciales son aceptados y persistirán sólo los campos presentes.

### Ejemplos de JSON de inserción

`POST /api/telemetry/system`
```json
{
	"timestamp": "2025-12-21T10:00:00Z",
	"type": "system",
	"cpuUsage": 42,
	"memoryFree": 230000,
	"uptime": 3600000,
	"taskCount": 18,
	"cpuTemp": 47.5
}
```

`POST /api/telemetry/power`
```json
{
	"timestamp": "2025-12-21T10:00:01Z",
	"type": "power",
	"voltage": 3.72,
	"current": 0.51,
	"solarVoltage": 5.0,
	"solarCurrent": 0.2,
	"batteryLevel": 86,
	"batteryTemp": 32
}
```

`POST /api/telemetry/temperature`
```json
{
	"timestamp": "2025-12-21T10:00:02Z",
	"type": "temperature",
	"obcTemp": 26.4,
	"commsTemp": 27.1,
	"payloadTemp": 25.9,
	"batteryTempFloat": 31.2,
	"externalTemp": 18.3
}
```

`POST /api/telemetry/comms`
```json
{
	"timestamp": "2025-12-21T10:00:03Z",
	"type": "comms",
	"rssi": -65,
	"snr": 9,
	"commsUptime": 123456,
	"successRate": 97
}
```

`POST /api/telemetry` (general)
```json
{
	"timestamp": "2025-12-21T10:00:04Z",
	"type": "general",
	"rawLine": "[General] Telemetry line"
}
```

### Cómo ejecutar (Maven) 🏃

Requisitos: Java 17, Maven.

```bash
cd backend
mvn spring-boot:run
# o empaquetar
mvn clean package -DskipTests
java -jar target/fomalhaut-backend-1.0.0.jar
```

El servidor quedará en `http://localhost:20001/api`.

### Cómo ejecutar (Docker) 🐳

Requisitos: Docker.

```bash
cd backend
docker build -t fomalhaut-backend:1.0 .
docker run --rm -p 20001:20001 fomalhaut-backend:1.0
```

### Consola H2 (debug)

- Activada en `application.properties`:
	- `spring.h2.console.enabled=true`
	- `spring.h2.console.path=/h2-console`
- Accede a `http://localhost:20001/h2-console`
- JDBC URL: `jdbc:h2:mem:testdb`, usuario `sa`, sin password.

### CORS y seguridad

- CORS abierto (`*`) para facilitar desarrollo del bridge y frontends.
- Para producción, ajustar orígenes permitidos en `FomalhautBackendApplication.corsConfigurer()`.

### Integración con el bridge

- El bridge debe apuntar a `http://localhost:20001` y usar rutas `/api/telemetry/...`.
- Ejemplos de endpoints del bridge en `bridge/config.json` → `server.endpoints`.

### Notas y mejoras futuras

- Persistencia actual es en memoria (H2). Para producción, configurar PostgreSQL/MySQL y `spring.jpa.hibernate.ddl-auto=update`.
- Validación de payloads (Bean Validation) y DTOs separados por tipo podrían mejorar robustez.
- Paginación en GETs con grandes volúmenes (usar `Pageable`).

