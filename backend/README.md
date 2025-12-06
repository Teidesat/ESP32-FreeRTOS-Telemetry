# 🚀 Fomalhaut Backend - Java Spring Boot

Backend REST API para el servidor de telemetría Fomalhaut. Recibe logs del puente Python ESP32 y los sirve al frontend React.

## 📋 Requisitos

- **Java 17+** (OpenJDK recomendado)
- **Maven 3.8+**
- **Puerto 20001** disponible

O alternativamente:
- **Docker** + **Docker Compose**

## 🔧 Instalación

### Opción 1: Ejecución Local (Sin Docker)

#### 1. Instalar Maven (si no lo tienes)

**Linux (Debian/Ubuntu):**
```bash
sudo apt update
sudo apt install maven
```

**Linux (Fedora/RHEL):**
```bash
sudo dnf install maven
```

**macOS:**
```bash
brew install maven
```

**Windows:**
Descarga desde https://maven.apache.org/download.cgi

#### 2. Compilar la aplicación

```bash
cd backend
mvn clean package
```

#### 3. Ejecutar el backend

```bash
java -jar target/fomalhaut-backend-1.0.0.jar
```

O usar Maven directamente:
```bash
mvn spring-boot:run
```

### Opción 2: Con Docker Compose (Recomendado ✅)

```bash
# Desde el directorio raíz
docker-compose up -d
```

Esto iniciará:
- Backend en `http://localhost:20001/api`
- Frontend en `http://localhost:20002`

## 📡 API Endpoints

### GET - Obtener Logs

**Todos los logs:**
```bash
curl http://localhost:20001/api/telemetry
```

**Solo sistema:**
```bash
curl http://localhost:20001/api/telemetry/system
```

**Solo potencia:**
```bash
curl http://localhost:20001/api/telemetry/power
```

**Solo temperatura:**
```bash
curl http://localhost:20001/api/telemetry/temperature
```

**Solo comunicaciones:**
```bash
curl http://localhost:20001/api/telemetry/comms
```

**Últimos N logs:**
```bash
curl http://localhost:20001/api/telemetry/latest/50
```

### POST - Enviar Logs (desde Bridge)

**Sistema:**
```bash
curl -X POST http://localhost:20001/api/telemetry/system \
  -H "Content-Type: application/json" \
  -d '{
    "timestamp": "2025-12-04T10:30:45.123456",
    "type": "system",
    "cpu_usage": 45,
    "ram_free": 234,
    "raw_line": "[Sistema] CPU: 45%, RAM: 234KB"
  }'
```

**Potencia:**
```bash
curl -X POST http://localhost:20001/api/telemetry/power \
  -H "Content-Type: application/json" \
  -d '{
    "timestamp": "2025-12-04T10:30:46.789012",
    "type": "power",
    "voltage": 3.7,
    "current": 0.5,
    "raw_line": "[Potencia] Voltaje: 3.7V, Corriente: 0.5A"
  }'
```

### DELETE - Limpiar Logs

**Eliminar todos:**
```bash
curl -X DELETE http://localhost:20001/api/telemetry/clear
```

**Eliminar por ID:**
```bash
curl -X DELETE http://localhost:20001/api/telemetry/123
```

## 🗄️ Base de Datos

El backend usa **H2 Database** (in-memory), lo que significa:
- ✅ No requiere instalación externa
- ✅ Rápido para development
- ⚠️ Los datos se pierden al reiniciar

### Para datos persistentes (futuro):

Edita `src/main/resources/application.properties`:
```properties
# Cambiar de H2 a PostgreSQL
spring.datasource.url=jdbc:postgresql://localhost:5432/fomalhaut
spring.datasource.username=postgres
spring.datasource.password=password
spring.jpa.database-platform=org.hibernate.dialect.PostgreSQLDialect
spring.jpa.hibernate.ddl-auto=update
```

## 📊 Logs de la Aplicación

Para ver los logs detallados:

```bash
# Durante compilación
mvn clean package -X

# Durante ejecución
java -jar target/fomalhaut-backend-1.0.0.jar --debug
```

## 🐛 Troubleshooting

### Error: "Port 20001 already in use"

Otro proceso está usando el puerto:

```bash
# Linux/macOS
lsof -i :20001

# Windows
netstat -ano | findstr :20001
```

Solución: Cambiar puerto en `application.properties`:
```properties
server.port=20002
```

### Error: "No compiler is provided"

Falta Java Development Kit:

```bash
# Linux
sudo apt install openjdk-17-jdk

# Verificar
java -version
javac -version
```

### Error de conexión desde Bridge

Verifica que el backend está corriendo:

```bash
curl http://localhost:20001/api/telemetry
```

Si no responde, revisa los logs de la aplicación.

## 🏗️ Estructura del Proyecto

```
backend/
├── pom.xml                          # Configuración Maven
├── Dockerfile                       # Imagen Docker
├── src/main/
│   ├── java/com/teidesat/fomalhaut/
│   │   ├── FomalhautBackendApplication.java  # Punto de entrada
│   │   ├── model/
│   │   │   └── Telemetry.java              # Entidad JPA
│   │   ├── repository/
│   │   │   └── TelemetryRepository.java    # DAO
│   │   └── controller/
│   │       └── TelemetryController.java    # REST API
│   └── resources/
│       └── application.properties          # Configuración
└── target/                          # Salida compilada
```

## 📝 Notas de Desarrollo

### Recompilar después de cambios:

```bash
mvn clean package
java -jar target/fomalhaut-backend-1.0.0.jar
```

O para desarrollo con auto-reload:

```bash
mvn spring-boot:run -Dspring-boot.run.arguments="--spring.devtools.restart.enabled=true"
```

### Acceder a la consola H2:

URL: http://localhost:20001/api/h2-console
- JDBC URL: `jdbc:h2:mem:testdb`
- Usuario: `sa`
- Contraseña: (vacía)

## 🚀 Despliegue en Producción

Para producción, se recomienda:

1. **Usar base de datos persistente** (PostgreSQL, MySQL)
2. **Configurar HTTPS/SSL**
3. **Añadir autenticación JWT**
4. **Configurar logging a archivo**
5. **Implementar monitoreo**

Edita `application-prod.properties` con configuración específica.

## 📞 Soporte

Para problemas con el backend:

1. Revisa los logs: `docker logs fomalhaut-backend`
2. Verifica la configuración: `application.properties`
3. Prueba endpoints con `curl` o Postman
4. Consulta la documentación de Spring Boot

## 📄 Licencia

GPL-3.0 - TeideSat Project

---

**Versión:** 1.0.0  
**Última actualización:** Diciembre 2025  
**Autor:** TeideSat Team
