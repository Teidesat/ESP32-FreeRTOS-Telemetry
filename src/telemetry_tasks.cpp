/**
 * @file telemetry_tasks.cpp
 * @brief Tareas FreeRTOS adicionales (no usadas en main.cpp actualmente)
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 20-10-2025
 * 
 * @details
 * Este archivo contiene tareas FreeRTOS adicionales que pueden usarse
 * en futuras implementaciones o configuraciones alternativas.
 * 
 * Las tareas principales se definen ahora en main.cpp:
 * - vTelemetryGeneratorTask: Genera datos de telemetría
 * - vTelemetryTransmitterTask: Transmite datos por serial
 * 
 * @note Las tareas vTelemetryCollectorTask y vTelemetryProcessorTask
 * se mantienen aquí como referencia para futuras expansiones.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "../include/telemetry_storage.h"
#include "../include/telemetry_generators.h"
#include "../include/telemetry_tasks.h"
#include "../include/telemetry_logger.h"
#include "../include/telemetry_acquisition.h"
#include "../include/telemetry_processing.h"

// Handles globales para diagnóstico de stack
TaskHandle_t gTaskCollectHandle = NULL;
TaskHandle_t gTaskProcessHandle = NULL;
TaskHandle_t gTaskTransmitHandle = NULL;

/**
 * @brief Tarea de recolección (NO USADA EN MAIN.CPP)
 * @note Mantiene la interfaz compatible con adquisición de datos
 */
void vTelemetryCollectorTask(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  telemetry_logf("🚀 Telemetry Collector Task Started");
  telemetry_acquisition_init();

  for(;;) {
    telemetry_acquisition_cycle();
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
  }
}

/**
 * @brief Tarea de procesamiento (NO USADA EN MAIN.CPP)
 * @note Mantiene la interfaz compatible con procesamiento de datos
 */
void vTelemetryProcessorTask(void *pvParameters) {
  telemetry_logf("🔧 Telemetry Processor Task Started");
  telemetry_processing_init();

  for(;;) {
    if(!telemetry_processing_handle_one()) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}