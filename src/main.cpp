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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "../include/telemetry_storage.h"
#include "../include/telemetry_logger.h"
#include "../include/telemetry_diagnostics.h"
#include "../include/telemetry_acquisition.h"
#include "../include/telemetry_processing.h"
#include "../include/telemetry_transmission.h"

// Declaración de las tareas de telemetría
void vTelemetryCollectorTask(void *pvParameters);
void vTelemetryProcessorTask(void *pvParameters);
void vTelemetryTransmitterTask(void *pvParameters);

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

  // Crear tareas de telemetría (guardar handles para diagnóstico de stack)
  extern TaskHandle_t gTaskCollectHandle;
  extern TaskHandle_t gTaskProcessHandle;
  extern TaskHandle_t gTaskTransmitHandle;

  xTaskCreate(
    vTelemetryCollectorTask,   // Función
    "TelemCollect",            // Nombre
    4096,                      // Stack size
    NULL,                      // Parámetros
    2,                         // Prioridad
    &gTaskCollectHandle        // Handle
  );

  xTaskCreate(
    vTelemetryProcessorTask,
    "TelemProcess", 
    4096,
    NULL,
    1,
    &gTaskProcessHandle
  );

  xTaskCreate(
    vTelemetryTransmitterTask,
    "TelemXmit",
    4096,
    NULL,
    1,
    &gTaskTransmitHandle
  );

  telemetry_logf("✅ All telemetry tasks created successfully");
  telemetry_logf("📡 System operational - Telemetry data generation started");
  telemetry_logf("--------------------------------------------------------");

  // Inicializar diagnóstico separado
  telemetry_diagnostics_init();
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
  // Delegar diagnóstico periódico al módulo de diagnóstico
  telemetry_diagnostics_tick();
  vTaskDelay(pdMS_TO_TICKS(1000));
}