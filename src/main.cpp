/**
 * @file main.cpp
 * @brief Sistema de Telemetría para Satélite TeideSat - Programa Principal
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 20-10-2025
 * 
 * @details
 * Este archivo contiene la implementación principal del sistema de telemetría
 * para el satélite TeideSat. El sistema está diseñado para ejecutarse en un
 * ESP32 y utiliza FreeRTOS para la gestión de tareas concurrentes.
 * 
 * El sistema implementa tres tareas principales:
 * - Recolector de telemetría: Captura datos de los sensores
 * - Procesador de telemetría: Procesa y analiza los datos capturados
 * - Transmisor de telemetría: Envía los datos procesados
 * 
 * @note Este código está optimizado para ejecutarse en un ESP32-WROOM-32.
 */

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "../include/telemetry_storage.h"
#include "../include/telemetry_logger.h"

// Declaración de las tareas de telemetría
void vTelemetryCollectorTask(void *pvParameters);
void vTelemetryProcessorTask(void *pvParameters);
void vTelemetryTransmitterTask(void *pvParameters);

// --- Ejemplo mínimo LittleFS: crear, escribir, leer y borrar ---
static void runLittleFSDemo() {
  Serial.println("\n[LittleFS] Demo: crear -> añadir -> leer -> borrar");

  if (!LittleFS.begin(true)) {
    Serial.println("[LittleFS] ERROR: no se pudo montar (formateado si era necesario)");
    return;
  }

  const char *path = "/demo.txt";

  // Limpieza inicial opcional
  if (LittleFS.exists(path)) {
    LittleFS.remove(path);
  }

  // 1) Crear y escribir
  {
    File f = LittleFS.open(path, FILE_WRITE);
    if (!f) {
      Serial.println("[LittleFS] ERROR al crear archivo");
      return;
    }
    f.println("Hola desde LittleFS 👋");
    f.println("Linea 1");
    f.close();
    Serial.println("[LittleFS] Archivo creado y escrito (2 lineas)");
  }

  // 2) Añadir (append)
  {
    File f = LittleFS.open(path, FILE_APPEND);
    if (!f) {
      Serial.println("[LittleFS] ERROR al abrir para append");
      return;
    }
    f.println("Linea 2 (append)");
    f.close();
    Serial.println("[LittleFS] Linea añadida (append)");
  }

  // 3) Leer contenido completo
  {
    File f = LittleFS.open(path, FILE_READ);
    if (!f) {
      Serial.println("[LittleFS] ERROR al abrir para lectura");
      return;
    }
    Serial.println("[LittleFS] Contenido de /demo.txt:");
    while (f.available()) {
      Serial.write(f.read());
    }
    f.close();
  }

  // 4) Borrar
  if (LittleFS.remove(path)) {
    Serial.println("\n[LittleFS] Archivo borrado correctamente");
  } else {
    Serial.println("\n[LittleFS] ERROR al borrar archivo");
  }
}

/**
 * @brief Función de inicialización del sistema
 * 
 * @details Esta función se ejecuta una vez al inicio y realiza:
 * - Inicialización de la comunicación serial
 * - Creación de las tareas de FreeRTOS
 * - Configuración inicial del sistema
 * 
 * Las tareas se crean con diferentes prioridades:
 * - Recolector: Prioridad 2 (alta)
 * - Procesador: Prioridad 1 (normal)
 * - Transmisor: Prioridad 1 (normal)
 */
void setup() {
  Serial.begin(115200);

  // Esperar un poco que Serial esté listo
  delay(1000);

  // Demo LittleFS antes de iniciar tareas
  runLittleFSDemo();

  // Inicializar logger y registrar algunas lineas al archivo
  telemetry_logger_init();

  // Borrar el contenido previo del log para esta sesión
  telemetry_log_clear();

  telemetry_logf("Sistema de telemetría iniciando...");
  // Escribe un identificador de arranque para poder ver claramente que proviene del fichero
  uint32_t bootId = esp_random();
  telemetry_logf("LOG PROOF: BOOT_ID=%08X", (unsigned)bootId);

  telemetry_logf("\n🛰️  TEIDESAT SATELLITE TELEMETRY SYSTEM - ESP32 WOKWI");
  telemetry_logf("======================================================");
  telemetry_logf("Starting FreeRTOS tasks...");

  // Crear tareas de telemetría
  xTaskCreate(
    vTelemetryCollectorTask,   // Función
    "TelemCollect",            // Nombre
    4096,                      // Stack size
    NULL,                      // Parámetros
    2,                         // Prioridad
    NULL                       // Handle
  );

  xTaskCreate(
    vTelemetryProcessorTask,
    "TelemProcess", 
    4096,
    NULL,
    1,
    NULL
  );

  xTaskCreate(
    vTelemetryTransmitterTask,
    "TelemXmit",
    4096,
    NULL,
    1,
    NULL
  );

  telemetry_logf("✅ All telemetry tasks created successfully");
  telemetry_logf("📡 System operational - Telemetry data generation started");
  telemetry_logf("--------------------------------------------------------");
}

/**
 * @brief Bucle principal del programa
 * 
 * @details En un sistema basado en FreeRTOS, el loop principal se utiliza
 * principalmente para tareas de mantenimiento y monitoreo. En este caso:
 * - Muestra estadísticas del sistema cada 30 segundos
 * - Monitorea el estado general del ESP32
 * - Reporta uso de memoria y número de tareas
 * 
 * @note La mayoría del trabajo real se realiza en las tareas de FreeRTOS,
 * no en este loop.
 */
void loop() {
  // FreeRTOS maneja las tareas, este loop puede estar vacío
  delay(1000);

  // Dump periódico del fichero cada 15 segundos
  static uint32_t last_dump = 0;
  if (millis() - last_dump > 15000) {
    Serial.println("\n[Logger] Dump periódico del fichero /telemetry_log.txt:");
    telemetry_dump_log();
    last_dump = millis();
  }

	// Opcional: mostrar estado general periódicamente
  static uint32_t last_status = 0;
  if(millis() - last_status > 30000) { // Cada 30 segundos
    last_status = millis();
    telemetry_logf("\n📈 SYSTEM STATUS: Uptime: %lus | Heap: %lu | Tasks: %d",
                   millis() / 1000,
                   esp_get_free_heap_size(),
                   uxTaskGetNumberOfTasks());
  }
}