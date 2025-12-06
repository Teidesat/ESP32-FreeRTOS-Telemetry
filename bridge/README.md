# 🌉 ESP32 → Fomalhaut Bridge

Script puente en Python que transmite los logs de telemetría del ESP32 al servidor local Java-Spring de Fomalhaut en tiempo real.

## 📋 Descripción

Este bridge actúa como intermediario entre:
- **ESP32**: Genera logs de telemetría por puerto serial
- **Servidor Fomalhaut**: Recibe datos vía API REST HTTP

### Características

✅ Lectura en tiempo real del puerto serial  
✅ Parsing inteligente de logs (Sistema, Potencia, Temperatura, Comms)  
✅ Envío automático vía HTTP POST al servidor local  
✅ Reintentos automáticos en caso de error  
✅ Estadísticas de transmisión  
✅ Configuración flexible vía JSON  

## 🚀 Instalación Rápida

### 1. Instalar dependencias

```bash
cd bridge
pip install -r requirements.txt
```

### 2. Configurar el bridge

Edita `config.json` según tu entorno:

```json
{
  "serial": {
    "port": "/dev/ttyUSB0",    // Cambia según tu puerto
    "baudrate": 115200
  },
  "server": {
    "base_url": "http://localhost:20001",  // URL del servidor Java-Spring
    "endpoints": {
      "telemetry": "/api/telemetry",
      "system": "/api/telemetry/system",
      "power": "/api/telemetry/power",
      "temperature": "/api/telemetry/temperature",
      "comms": "/api/telemetry/comms"
    }
  },
  "debug": true
}
```

### 3. Ejecutar el bridge

```bash
python3 esp32_to_fomalhaut_bridge.py
```

## 🔧 Configuración Detallada

### Encontrar el puerto serial del ESP32

**Linux:**
```bash
# Listar puertos disponibles
ls /dev/tty*

# Ver información de dispositivos USB
dmesg | grep tty

# Común: /dev/ttyUSB0, /dev/ttyACM0
```

**Windows:**
```
Administrador de dispositivos → Puertos (COM y LPT)
# Común: COM3, COM4, COM5
```

**macOS:**
```bash
ls /dev/cu.*
# Común: /dev/cu.usbserial-XXXXX
```

### Ajustar endpoints del servidor

```json
{
  "telemetry": "/api/v1/telemetry/data",
  "system": "/api/v1/telemetry/system",
  "power": "/api/v1/telemetry/power",
  "temperature": "/api/v1/telemetry/temperature",
  "comms": "/api/v1/telemetry/comms"
}
```

## 📊 Formato de Datos

El bridge parsea automáticamente logs del ESP32 y los estructura en JSON:

### Ejemplo: Telemetría de Sistema

**ESP32 genera:**
```
[Sistema] CPU: 45%, RAM: 234KB, Stack libre: 2048
```

**Bridge envía:**
```json
{
  "timestamp": "2025-12-01T10:30:45.123456",
  "type": "system",
  "cpu_usage": 45,
  "ram_free": 234,
  "raw_line": "[Sistema] CPU: 45%, RAM: 234KB, Stack libre: 2048"
}
```

### Ejemplo: Telemetría de Potencia

**ESP32 genera:**
```
[Potencia] Voltaje: 3.7V, Corriente: 0.5A
```

**Bridge envía:**
```json
{
  "timestamp": "2025-12-01T10:30:46.789012",
  "type": "power",
  "voltage": 3.7,
  "current": 0.5,
  "raw_line": "[Potencia] Voltaje: 3.7V, Corriente: 0.5A"
}
```

## 🎯 Uso Típico

### Workflow completo

1. **Iniciar el servidor backend de Fomalhaut**
   ```bash
   cd Fomalhaut
   ```

2. **Subir código al ESP32**
   ```bash
   cd /home/u/Teidesat/probando
   platformio run --target upload
   ```

3. **Iniciar el bridge**
   ```bash
   cd bridge
   python3 esp32_to_fomalhaut_bridge.py
   ```

4. **Ver logs en tiempo real**
   - El bridge mostrará cada línea leída del ESP32
   - Confirmará cuando envíe datos al servidor
   - Mostrará errores si el servidor no responde

### Salida típica del bridge

```
============================================================
🛰️  ESP32 → FOMALHAUT BRIDGE
============================================================
📡 Puerto Serial: /dev/ttyUSB0
🌐 Servidor: http://localhost:8080
🐛 Debug: ON
============================================================

✅ Conectado a /dev/ttyUSB0 @ 115200 baud
🚀 Bridge iniciado. Leyendo datos del ESP32...
   (Presiona Ctrl+C para detener)

📥 [Sistema] CPU: 45%, RAM: 234KB
✅ [system] Enviado → 200
📥 [Potencia] Voltaje: 3.7V, Corriente: 0.5A
✅ [power] Enviado → 200
📥 [Temperatura] Sensor1: 25.3°C
✅ [temperature] Enviado → 200
```

## 🐛 Troubleshooting

### Error: "No se pudo conectar al servidor"

```
⚠️  No se pudo conectar al servidor en http://localhost:20001
💡 Verifica que el servidor Java-Spring esté ejecutándose
```

**Solución:** Asegúrate de que el backend de Fomalhaut está corriendo antes de iniciar el bridge.

### Error: "Permission denied" en puerto serial

```bash
# Linux: Añadir usuario al grupo dialout
sudo usermod -a -G dialout $USER
# Cerrar sesión y volver a iniciar

# O dar permisos temporales
sudo chmod 666 /dev/ttyUSB0
```

### Error: "Device or resource busy"

Otro programa está usando el puerto serial (como el monitor de PlatformIO).

```bash
# Cerrar otros monitores seriales
# Verificar qué proceso usa el puerto
lsof | grep ttyUSB0
```

## 🤝 Integración con Backend

El equipo de backend de Fomalhaut debe implementar endpoints que reciban el JSON. Ejemplo en Spring Boot:

## 📚 Referencias

- [Proyecto TeideSat](https://github.com/Teidesat)
- [Fomalhaut - Frontend](https://github.com/Teidesat/Fomalhaut)
- [PySerial Documentation](https://pyserial.readthedocs.io/)
- [Requests Library](https://requests.readthedocs.io/)

## 📄 Licencia

GPL-3.0