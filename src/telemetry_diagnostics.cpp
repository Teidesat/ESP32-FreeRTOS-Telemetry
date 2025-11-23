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

static uint32_t s_last_dump_ms = 0;
static uint32_t s_last_status_ms = 0;

void telemetry_diagnostics_init(void) {
  s_last_dump_ms = millis();
  s_last_status_ms = millis();
  telemetry_logf("[DIAG] Init OK");
}

void telemetry_diagnostics_tick(void) {
  uint32_t now = millis();

  // Dump periódico del fichero cada 30 s
  if (now - s_last_dump_ms > 30000) {
    telemetry_logf("\n[DIAG] File dump trigger");
    telemetry_dump_log();
    s_last_dump_ms = now;
  }

  // Estado del sistema cada 20 s
  if (now - s_last_status_ms > 20000) {
    s_last_status_ms = now;
    uint32_t written, read, lost;
    telemetry_get_stats(&written, &read, &lost);
    telemetry_logf("\n📈 SYSTEM STATUS: Uptime: %lus | Heap: %lu | Tasks: %d | Buf W/R/L=%lu/%lu/%lu",\
                    now / 1000, esp_get_free_heap_size(), uxTaskGetNumberOfTasks(), written, read, lost);
  }
}
