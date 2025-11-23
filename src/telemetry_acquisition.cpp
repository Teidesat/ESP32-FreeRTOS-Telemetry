/**
 * @file telemetry_acquisition.cpp
 * @brief Implementación del módulo de adquisición de telemetría (wrapper de generadores)
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 22-11-2025
 *
 * @details
 * Este módulo inicializa el almacenamiento de telemetría y coordina la generación
 * de diferentes tipos de datos de telemetría mediante llamadas a los generadores específicos.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../include/telemetry_acquisition.h"
#include "../include/telemetry_generators.h"
#include "../include/telemetry_storage.h"
#include "../include/telemetry_logger.h"

void telemetry_acquisition_init(void) {
  telemetry_storage_init();
  telemetry_logf("[ACQ] Init OK");
}

void telemetry_acquisition_cycle(void) {
  // Generar todos los tipos de datos (orden puede afectar prioridad futura)
  generate_system_telemetry();
  generate_power_telemetry();
  generate_temperature_telemetry();
  generate_subsystem_telemetry();
}
