## Bridge ESP32 → Fomalhaut

Pequeño puente en Python que lee líneas JSON desde el puerto serie del ESP32 y las reenvía al backend local (Java/Spring) de Fomalhaut.

### Contenido del directorio

- `bridge.py`: Script principal del bridge. Abre el puerto serie, filtra solo líneas que sean JSON válido y las publica en el backend según su tipo (`system`, `power`, `temperature`, `comms`). Añade un `timestamp` local y muestra estadísticas.
- `config.json`: Configuración del puente.
  - `serial.port`: Puerto del ESP32 (ej. `/dev/ttyUSB0` o `/dev/ttyACM0`).
  - `serial.baudrate`: Velocidad serie (por defecto `115200`).
  - `serial.timeout`: Timeout de lectura (segundos).
  - `server.base_url`: URL base del backend (por defecto `http://localhost:20001`).
  - `server.endpoints`: Rutas por tipo de telemetría.
  - `server.timeout`: Timeout de las peticiones HTTP.
  - `debug`, `retry_attempts`, `retry_delay`: flags simples para futura lógica de reintentos.
- `requirements.txt`: Dependencias Python (`pyserial`, `requests`, opcional `ipython`).
- `start_bridge.sh`: Script rápido que verifica Python y dependencias y ejecuta `bridge.py`.
- `test_direct.py`: Envío directo de telemetrías de ejemplo al backend (no usa el puerto serie). Útil para probar el backend sin hardware.
- `validate_setup.sh`: Script de validación (Python, dependencias, puertos, backend). para ejecutar, usa `start_bridge.sh` o `bridge.py` directamente.

### Requisitos

- Linux/macOS/Windows con Python 3.8+
- Backend de Fomalhaut corriendo en `server.base_url` (por defecto `http://localhost:20001`).
- Permisos para acceder al puerto serie en Linux (grupo `dialout` o udev rule).

### Pasos para probar el bridge (ESP32 real)

1. Conecta el ESP32 por USB y localiza el puerto:
	- Linux: `ls /dev/ttyUSB*` o `ls /dev/ttyACM*`
2. Ajusta `config.json`:
	- Cambia `serial.port` al dispositivo detectado (ej. `/dev/ttyUSB0`).
	- Verifica `server.base_url` apunta a tu backend.
3. Instala dependencias:
	```bash
	cd /home/u/ESP32-FreeRTOS-Telemetry/bridge
	python3 -m pip install -r requirements.txt
	```
4. (Opcional) Comprueba conectividad del backend:
	```bash
	curl -s --connect-timeout 3 http://localhost:20001 >/dev/null && echo "Backend OK" || echo "Backend no responde"
	```
5. Ejecuta el bridge:
	```bash
	./start_bridge.sh
	# o directamente
	python3 bridge.py
	```
6. Observa el output. Deberías ver líneas tipo `✅ [SYSTEM] → 201` indicando envíos exitosos.

### Prueba rápida sin hardware (solo backend)

1. Asegúrate de que el backend está corriendo en `config.json` → `server.base_url`.
2. Ejecuta el emulador sencillo:
	```bash
	python3 test_direct.py
	```
3. Verás respuestas HTTP (200/201) y un resumen de datos recuperados del backend.

### Notas y solución de problemas

- Permisos de puerto serie (Linux): si ves `serial.SerialException: Permission denied`, añade tu usuario al grupo `dialout` y vuelve a iniciar sesión:
  ```bash
  sudo usermod -a -G dialout "$USER"
  ```
- Backend no responde: revisa que `server.base_url` sea correcto y el servicio esté levantado. Prueba con `curl` como arriba.
- Formato de líneas del ESP32: el bridge solo procesa JSON que empiece por `{`. Las líneas no JSON se ignoran (contador `non_json_ignored`).
- Tipos admitidos: `system`, `power`, `temperature`, `comms`. Otros tipos se descartan.
- Salida y estadísticas: al terminar con `Ctrl+C`, se imprimen totales de líneas leídas, JSON parseados y paquetes enviados.

---

Siguientes mejoras sugeridas:
- Actualizar `validate_setup.sh` para usar `bridge.py` y `test_direct.py`.
- Añadir reintentos configurables usando `retry_attempts` y `retry_delay` del `config.json`.
