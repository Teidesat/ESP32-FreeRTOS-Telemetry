/**
 * @file telemetry_tasks.c
 * @brief Tareas FreeRTOS del sistema de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 20-10-2025
 * 
 * @details
 * Este archivo contiene la implementación de las tareas FreeRTOS que componen
 * el sistema de telemetría del satélite TeideSat.
 * 
 * El sistema está compuesto por tres tareas principales que ejecutan
 * concurrentemente:
 * - Recolector: Genera y almacena datos de telemetría
 * - Procesador: Procesa y visualiza los datos almacenados
 * - Transmisor: Simula el envío de datos a estación terrestre
 * 
 * @note Las tareas están optimizadas para entorno WOKWI con intervalos
 * reducidos para facilitar la visualización durante pruebas.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "../include/telemetry_storage.h"
#include "../include/telemetry_generators.h"
#include "../include/telemetry_tasks.h"
#include "../include/telemetry_logger.h"
// Handles globales para diagnóstico de stack
TaskHandle_t gTaskCollectHandle = NULL;
TaskHandle_t gTaskProcessHandle = NULL;
TaskHandle_t gTaskTransmitHandle = NULL;
#include "../include/telemetry_acquisition.h"
#include "../include/telemetry_processing.h"
#include "../include/telemetry_transmission.h"


void vTelemetryCollectorTask(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  telemetry_logf("🚀 Telemetry Collector Task Started");
  telemetry_acquisition_init();

  for(;;) {
    telemetry_acquisition_cycle();
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000)); // 2 segundos (antes era 9)
  }
}


void vTelemetryProcessorTask(void *pvParameters) {
  telemetry_logf("🔧 Telemetry Processor Task Started");
  telemetry_processing_init();

  for(;;) {
    if(!telemetry_processing_handle_one()) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

void vTelemetryTransmitterTask(void *pvParameters) {
  telemetry_logf("📡 Telemetry Transmitter Task Started");
  telemetry_transmission_init();
  for(;;) {
    telemetry_transmission_cycle();
    vTaskDelay(pdMS_TO_TICKS(2000));
  }

  // Crear tareas desde un punto común usando handles
  // Nota: Este archivo no define setup(), pero las tareas se crean en main.cpp.
}