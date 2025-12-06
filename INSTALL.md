# 🚀 GUÍA: ESP32 → FOMALHAUT (Logs en tiempo real)

## ⚙️ Prerequisitos

```bash
java -version       # Java 17+
mvn -v              # Maven 3.8+
python3 --version   # Python 3.9+
docker --version    # Docker + Docker Compose
```

---

## 🐳 OPCIÓN 1: CON DOCKER (Recomendado)

### Paso 1: Compilar Backend

```bash
cd backend
mvn clean package
cd ..
```

### Paso 2: Compilar Frontend

```bash
cd Fomalhaut
npm install
npm run build
cd ..
```

### Paso 3: Iniciar con Docker Compose

```bash
# Limpiar contenedores antiguos (si existen)
docker-compose down

# Iniciar
docker-compose up -d
```

✅ Espera 10 segundos

### Paso 4: Verificar servicios

```bash
docker ps
# Deberías ver: fomalhaut-backend (20001) y fomalhaut-frontend (20002)
```

### Paso 5: Abre Fomalhaut en navegador

```
http://localhost:20002
```

### Paso 6: Ejecutar Bridge Python

Terminal nueva:
```bash
cd bridge
pip install -r requirements.txt
python3 esp32_to_fomalhaut_bridge.py
```

### Paso 7: Ver logs en Fomalhaut

En el navegador: pestaña **"Logs"** → verás datos del ESP32 en tiempo real

✅ ¡Listo!

---

## 💻 OPCIÓN 2: SIN DOCKER (Local)

### Paso 1: Backend

Terminal 1:
```bash
cd backend
mvn clean package
java -jar target/fomalhaut-backend-1.0.0.jar
```

### Paso 2: Frontend

Terminal 2:
```bash
cd Fomalhaut
npm install
npm run dev
```

Abre: `http://localhost:5173`

### Paso 3: Bridge Python

Terminal 3:
```bash
cd bridge
pip install -r requirements.txt
python3 esp32_to_fomalhaut_bridge.py
```

---

## 🔌 Configurar ESP32 (Ambas opciones)

1. **Conecta ESP32 por USB**

2. **Busca el puerto:**
   ```bash
   ls /dev/ttyUSB* /dev/ttyACM*
   ```

3. **Edita `bridge/config.json`:**
   ```json
   {
     "serial": {
       "port": "/dev/ttyUSB0",
       "baudrate": 115200
     }
   }
   ```

---

## 🧪 Testing sin ESP32

```bash
cd bridge
python3 test_direct.py
```

---

## ⚠️ Problemas

| Problema | Solución |
|----------|----------|
| Container ya existe | `docker-compose down` |
| "Connection refused" Backend | Verifica si está corriendo: `docker ps` o `ps aux \| grep java` |
| "Permission denied" ESP32 | `sudo usermod -a -G dialout $USER` + logout/login |
| "pyserial not found" | `pip install -r bridge/requirements.txt` |
| Port 5173 en uso (local) | `lsof -i :5173` y mata el proceso |

---

## 🎯 Flujo de datos

```
ESP32 (Serial USB)
  ↓
Bridge Python (lee puerto → HTTP)
  ↓
Backend Java (BD + API)
  ↓
Frontend Fomalhaut (Dashboard)
```

```bash
cd bridge
python3 test_direct.py
```

Envía datos de prueba directamente al Backend.