# ✅ GUÍA DE INSTALACIÓN PASO A PASO

Sigue estos pasos para ver los logs del ESP32 en el Dashboard de Fomalhaut.

## 🎯 Objetivo Final

Ver en tiempo real los logs del ESP32 en la pestaña **"Logs"** del Frontend de Fomalhaut en tu navegador.

```
tu-navegador:20002 → Frontend React → Backend Java (20001) ← Bridge Python ← ESP32 (Serial)
```

---

## 📋 Requisitos

- **Java 17+** - [Instalar](https://www.oracle.com/java/technologies/downloads/#java17)
- **Maven 3.8+** - [Instalar](https://maven.apache.org/download.cgi)
- **Python 3.9+** - [Instalar](https://www.python.org/downloads/)
- **Git** - (probablemente ya lo tienes)
- **Docker + Docker-Compose** (opcional pero recomendado)

### Verificar que tienes todo

```bash
java -version          # Debe ser 17+
mvn -v                 # Debe ser 3.8+
python3 --version      # Debe ser 3.9+
docker --version       # Opcional
```

---

## 🚀 MÉTODO 1: Docker Compose (Recomendado) ⭐

**Ventajas:** Más fácil, todo automático  
**Requisito:** Docker instalado

### Paso 1: Navega al directorio

```bash
cd /home/u/Teidesat/probando
```

### Paso 2: Inicia todo con Docker Compose

```bash
docker-compose up -d
```

Espera 10-15 segundos para que todo esté listo.

### Paso 3: Verifica que está corriendo

```bash
# Backend debe responder
curl http://localhost:20001/api/telemetry

# Debe mostrar: []  (lista vacía de logs)
```

### Paso 4: Abre el navegador

```
http://localhost:20002
```

✅ ¡El Frontend está corriendo!

### Paso 5: Genera datos de prueba

```bash
./demo.sh
```

Esto:
1. Limpia logs previos
2. Simula un ESP32 generando datos
3. Los envía al Backend
4. Muestra estadísticas en tiempo real

### Paso 6: Haz clic en "Logs" en el Frontend

Ya deberías ver los logs aparecer en tiempo real con:
- 💻 Datos de Sistema
- 🔋 Datos de Potencia
- 🌡️ Datos de Temperatura
- 📡 Datos de Comunicaciones

---

## 🖥️ MÉTODO 2: Ejecución Local (Sin Docker)

**Requisitos:** Java, Maven, Python3

### Paso 1: Compilar el Backend

```bash
cd backend
mvn clean package
```

Esto tomará 1-2 minutos la primera vez (descargará dependencias).

### Paso 2: Ejecutar el Backend

```bash
java -jar target/fomalhaut-backend-1.0.0.jar
```

Deberías ver algo como:
```
Started FomalhautBackendApplication in X.XXX seconds
```

### Paso 3: Ejecutar el Frontend (Nueva Terminal)

```bash
cd Fomalhaut
npm install
npm run dev
```

Deberías ver:
```
VITE v6.0.5  ready in XXX ms

➜  Local:   http://localhost:5173/
```

### Paso 4: Instalar dependencias Python del Bridge

```bash
cd bridge
pip install -r requirements.txt
```

### Paso 5: Generar datos de prueba (Nueva Terminal)

```bash
cd bridge
python3 simulate_esp32.py | python3 esp32_to_fomalhaut_bridge.py &
```

Verás logs como:
```
[12:30:45] [Sistema] CPU: 45%, RAM: 234KB
✅ [system] Enviado → 200
```

### Paso 6: Abre el Frontend

```
http://localhost:5173/
```

O si está en puerto 20002:
```
http://localhost:20002/
```

---

## 📱 Con ESP32 Real Conectado

Si tienes un ESP32 físico conectado por USB:

### Paso 1: Asegúrate que el Backend está corriendo

```bash
curl http://localhost:20001/api/telemetry
# Debe responder sin errores
```

### Paso 2: Encuentra el puerto del ESP32

**Linux:**
```bash
ls /dev/ttyUSB*
ls /dev/ttyACM*
```

Típicamente: `/dev/ttyUSB0` o `/dev/ttyACM0`

**Windows:** Abre Device Manager y busca "COM3", "COM4", etc.

### Paso 3: Configura el Bridge

```bash
cd bridge
nano config.json
```

Cambia:
```json
{
  "serial": {
    "port": "/dev/ttyUSB0"  ← Reemplaza con tu puerto
  }
}
```

Guarda con `Ctrl+X`, `Y`, `Enter`

### Paso 4: Ejecuta el Bridge

```bash
cd bridge
python3 esp32_to_fomalhaut_bridge.py
```

Verás:
```
✅ Conectado a /dev/ttyUSB0 @ 115200 baud
🚀 Bridge iniciado. Leyendo datos del ESP32...
```

### Paso 5: Ver en Frontend

Los logs aparecerán automáticamente en:
```
http://localhost:20002/
→ Haz clic en "Logs" tab
```

---

## 🧪 Testing sin Hardware

Si no tienes ESP32 conectado, prueba con el simulador:

```bash
# Terminal 1: Backend corriendo (ya iniciado)

# Terminal 2: Simulador
cd bridge
python3 simulate_esp32.py

# Terminal 3: Bridge
cd bridge
python3 esp32_to_fomalhaut_bridge.py
```

O todo en uno:
```bash
cd bridge
timeout 60 python3 simulate_esp32.py | python3 esp32_to_fomalhaut_bridge.py
```

---

## 📊 Ver Datos en el Frontend

### Pestaña "Logs"

1. Abre http://localhost:20002 en tu navegador
2. Haz clic en **"Logs"** en el menú superior
3. Deberías ver una tabla con:
   - **Timestamp** - Hora del log
   - **Type** - Tipo (💻 system, 🔋 power, 🌡️ temp, 📡 comms)
   - **Datos** - CPU %, Voltaje, Temperatura, etc.
   - **Raw** - Línea original del ESP32

### Filtrar por tipo

Usa el dropdown "Filtrar por tipo" para ver solo:
- Sistema
- Potencia
- Temperatura
- Comunicaciones

### Auto Refresh

Activa "Auto Refresh" para que se actualice automáticamente cada 2 segundos.

---

## 🔍 Verificar Datos vía API

Si algo no funciona, verifica manualmente:

```bash
# Ver todos los logs
curl http://localhost:20001/api/telemetry | jq

# Ver solo sistema
curl http://localhost:20001/api/telemetry/system | jq

# Ver últimos 10
curl http://localhost:20001/api/telemetry/latest/10 | jq
```

---

## 🚨 Solucionar Problemas

### Error: "Connection refused" al conectar al Backend

```bash
# Verificar que Backend está corriendo
ps aux | grep java

# Si no está corriendo:
cd backend
java -jar target/fomalhaut-backend-1.0.0.jar
```

### Error: "Port 20001 already in use"

```bash
# Ver qué ocupa el puerto
lsof -i :20001

# Matar el proceso
kill -9 <PID>

# O usar otro puerto
sed -i 's/20001/20002/g' backend/src/main/resources/application.properties
```

### Error: "Cannot find pyserial"

```bash
cd bridge
pip install pyserial requests
```

### ESP32 no se conecta

```bash
# Verifica el puerto
ls -la /dev/ttyUSB*

# Verifica permisos
sudo usermod -a -G dialout $USER

# Logout y login para aplicar cambios
```

---

## ✅ Checklist Final

- [ ] Backend corriendo en puerto 20001
- [ ] Frontend corriendo en puerto 20002
- [ ] Puedes acceder a http://localhost:20002
- [ ] Bridge Python está iniciado
- [ ] Datos llegando al Backend
- [ ] Logs visibles en Frontend → Logs tab
- [ ] Filtros funcionando correctamente

---

## 🎯 Pasos Siguientes

1. **Conecta un ESP32 real**
   - Asegúrate que está subido el código de telemetría
   - Configura el puerto en `bridge/config.json`

2. **Monitorea datos en tiempo real**
   - Usa el Dashboard para ver métricas
   - Aplica filtros por tipo

3. **Personaliza la visualización**
   - Edita `Fomalhaut/src/environments/environment.ts` para endpoints diferentes
   - Modifica `bridge/config.json` para otros servidores

4. **Deploying a Producción**
   - Ver `backend/README.md` para configuración de producción
   - Usar PostgreSQL en lugar de H2
   - Configurar HTTPS

---

## 📞 Ayuda

### Ver logs de cada componente

```bash
# Backend logs
docker logs -f fomalhaut-backend

# Frontend en desarrollo
# Se muestra en la terminal donde ejecutaste npm run dev

# Bridge Python
# Se muestra en la terminal donde ejecutaste el script
```

### Resetear todo

```bash
# Docker
docker-compose down
docker-compose up -d

# Local
pkill -f "java.*fomalhaut"
pkill -f "python3.*bridge"
# Reinicia manualmente
```

### Limpiar datos

```bash
# Backend
curl -X DELETE http://localhost:20001/api/telemetry/clear

# Base de datos completa
docker-compose down -v
docker-compose up -d
```

---

## 🎓 Próximos Pasos de Aprendizaje

1. **Entender el flujo de datos:** [bridge/ARCHITECTURE.md](bridge/ARCHITECTURE.md)
2. **Detalles del Backend:** [backend/README.md](backend/README.md)
3. **Configuración del Bridge:** [bridge/README.md](bridge/README.md)
4. **Especificación API:** [bridge/BACKEND_SPEC.md](bridge/BACKEND_SPEC.md)

---

**¿Todo listo?** 🚀

Ve a: **http://localhost:20002** → Click en **"Logs"** → ¡Disfruta viendo los datos en tiempo real!

---

*Última actualización: Diciembre 2025*  
*TeideSat Project*
