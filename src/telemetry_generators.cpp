/**
 * @file telemetry_generators.c
 * @brief Generadores de datos de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 30-10-2025
 * 
 * @details
 * Este archivo contiene las funciones generadoras de datos de telemetría
 * para el sistema del satélite TeideSat. Las funciones simulan la lectura
 * de sensores y generan datos de telemetría estructurados que representan
 * el estado actual de los diferentes subsistemas del satélite.
 * 
 * El sistema genera cuatro tipos principales de telemetría:
 * - Estado del sistema (uptime, memoria, tareas)
 * - Datos de potencia (voltaje, corriente, nivel de batería)
 * - Temperaturas (OBC, comunicaciones, payload, batería, externa)
 * - Estado de subsistemas (comms, ADCS, payload, potencia)
 * 
 * @note En entorno este entorno de pruebas, no se utilizan sensores 
 * físicos reales, sino que se generan datos aleatorios realistas. 
 * En hardware real, estas funciones se modificarían para leer sensores físicos reales.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include <Arduino.h>
#include <ESPCPUTemp.h>
#include "../include/telemetry_storage.h"

static uint16_t sequence_number = 0; /**< Contador de secuencia para paquetes de telemetría */
// Contador de ciclos de generación (se mantiene para modelos de degradación como batería)
static uint32_t generation_cycle_count = 0; 

void generate_system_telemetry(void) {
  system_status_telem_t system_telem;

  system_telem.header.type = TELEM_SYSTEM_STATUS;
  system_telem.header.timestamp = xTaskGetTickCount();
  system_telem.header.sequence = sequence_number++;
  system_telem.header.priority = 1;

  // Uptime real basado en ticks FreeRTOS
  uint32_t uptime_sec = (uint32_t)(xTaskGetTickCount() / configTICK_RATE_HZ);
  system_telem.uptime_seconds = uptime_sec;
  generation_cycle_count++;

  // Estados específicos del ESP32
  system_telem.system_mode = 1; // nominal
  
  // CPU usage: 35-55% (simulado, como en main.cpp original)
  system_telem.cpu_usage = 35 + (esp_random() % 20);
  
  system_telem.stack_high_water = uxTaskGetStackHighWaterMark(NULL);

  // Memoria ESP32
  system_telem.heap_free = esp_get_free_heap_size();
  system_telem.task_count = uxTaskGetNumberOfTasks();

  // Temperatura CPU ESP32 (real)
  system_telem.cpu_temperature = temperatureRead();
  
  telemetry_store_packet((telemetry_packet_t*)&system_telem);
}


void generate_power_telemetry(void) {
  power_telem_t power_telem;

  power_telem.header.type = TELEM_POWER_DATA;
  power_telem.header.timestamp = xTaskGetTickCount();
  power_telem.header.sequence = sequence_number++;
  power_telem.header.priority = 2;

  // Voltaje de batería: 3.25-3.35V (como en main.cpp original)
  float voltage = 3.25f + ((esp_random() % 100) / 1000.0f);
  power_telem.battery_voltage = voltage;
  
  // Corriente: 0.45-0.55A (como en main.cpp original)
  float current = 0.45f + ((esp_random() % 100) / 1000.0f);
  power_telem.battery_current = current;
  
  // Voltaje solar: 4.8-5.2V (como en main.cpp original)
  float solar_voltage = 4.8f + ((esp_random() % 40) / 100.0f);
  power_telem.solar_panel_voltage = solar_voltage;
  
  // Corriente solar: 0.15-0.25A (como en main.cpp original)
  float solar_current = 0.15f + ((esp_random() % 100) / 1000.0f);
  power_telem.solar_panel_current = solar_current;
  
  // Nivel de batería: 80-90% (como en main.cpp original)
  int battery_level = 80 + (esp_random() % 11);
  power_telem.battery_level = battery_level;
  
  // Temperatura batería: 22-28°C (como en main.cpp original)
  int8_t battery_temp = 22 + (esp_random() % 7);
  power_telem.battery_temperature = battery_temp;
  
  power_telem.power_state = 0;

  telemetry_store_packet((telemetry_packet_t*)&power_telem);
}


void generate_temperature_telemetry(void) {
  // Las temperaturas se envían directamente desde vTelemetryGeneratorTask
  // Esta función no hace nada para evitar almacenar datos corruptos en storage
  // que luego serían transmitidos por vTelemetryTransmitterTask
}

void generate_subsystem_telemetry(void) {
  subsystem_status_telem_t subsys_telem;

  subsys_telem.header.type = TELEM_COMMUNICATION_STATUS;
  subsys_telem.header.timestamp = xTaskGetTickCount();
  subsys_telem.header.sequence = sequence_number++;
  subsys_telem.header.priority = 1;

  subsys_telem.comms_status = 1;
  subsys_telem.adcs_status = 1;  
  subsys_telem.payload_status = 1;
  subsys_telem.power_status = 1;
  
  uint32_t uptime_sec = (uint32_t)(xTaskGetTickCount() / configTICK_RATE_HZ);
  subsys_telem.comms_uptime = uptime_sec;
  subsys_telem.payload_uptime = (uptime_sec > 100) ? (uptime_sec - 100) : 0;
  subsys_telem.last_command_id = 0x25;
  
  // Success rate: 92-98% (como en main.cpp original)
  int success_rate = 92 + (esp_random() % 7);
  subsys_telem.command_success_rate = (uint8_t)success_rate;

  telemetry_store_packet((telemetry_packet_t*)&subsys_telem);
}