/**
 * @file telemetry_diagnostics.cpp
 * @brief Implementación del módulo de diagnóstico y salud del sistema de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 22-11-2025
 *
 * @details
 * Proporciona funciones para el diagnóstico periódico del sistema de telemetría,
 * incluyendo volcados de logs y reportes de estado del sistema.
 */
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "../include/telemetry_diagnostics.h"
#include "../include/telemetry_logger.h"
#include "../include/telemetry_storage.h"
#include "../include/telemetry_tasks.h"

static uint32_t s_last_dump_ms = 0;
static uint32_t s_last_status_ms = 0;

void telemetry_diagnostics_init(void) {
  s_last_dump_ms = millis();
  s_last_status_ms = millis();
  telemetry_logf("[DIAG] Init OK");
}

void telemetry_diagnostics_tick(void) {
  uint32_t now = millis();

  // Dump periódico de todos los ficheros de telemetría cada 45 s
  if (now - s_last_dump_ms > 45000) {
    telemetry_logf("\n[DIAG] Triggering periodic dump of all telemetry logs");
    telemetry_dump_all_logs();
    s_last_dump_ms = now;
  }

  // Reporte de uso de stack de tareas cada ~20s (solo si DEBUG_STACK está definido)
#ifdef DEBUG_STACK
  if (now - s_last_status_ms > 20000) {
    s_last_status_ms = now;
    if (gTaskCollectHandle) {
      UBaseType_t hwm = uxTaskGetStackHighWaterMark(gTaskCollectHandle);
      telemetry_logf("[STACK] TelemCollect high-water mark: %u stack words free", (unsigned)(hwm * sizeof(StackType_t)));
    }
    if (gTaskProcessHandle) {
      UBaseType_t hwm = uxTaskGetStackHighWaterMark(gTaskProcessHandle);
      telemetry_logf("[STACK] TelemProcess high-water mark: %u stack words free", (unsigned)(hwm * sizeof(StackType_t)));
    }
    if (gTaskTransmitHandle) {
      UBaseType_t hwm = uxTaskGetStackHighWaterMark(gTaskTransmitHandle);
      telemetry_logf("[STACK] TelemXmit high-water mark: %u stack words free", (unsigned)(hwm * sizeof(StackType_t)));
    }
  }
#endif
}