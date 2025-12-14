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

  // Uptime real basado en ticks FreeRTOS (configTICK_RATE_HZ normalmente = 1000 en Arduino ESP32)
  uint32_t uptime_sec = (uint32_t)(xTaskGetTickCount() / configTICK_RATE_HZ);
  system_telem.uptime_seconds = uptime_sec;
  generation_cycle_count++; // Incrementar ciclo de generación independiente del uptime real

  // Estados específicos del ESP32
  system_telem.system_mode = 1; // nominal
  system_telem.cpu_usage = 0;   // En ESP32 no tenemos esta métrica fácil
  system_telem.stack_high_water = uxTaskGetStackHighWaterMark(NULL);

  // Memoria ESP32
  system_telem.heap_free = esp_get_free_heap_size();
  system_telem.task_count = uxTaskGetNumberOfTasks();

  // temperatura CPU ESP32
  system_telem.cpu_temperature = temperatureRead();
  
  telemetry_store_packet((telemetry_packet_t*)&system_telem);
}


void generate_power_telemetry(void) {
  power_telem_t power_telem;

  power_telem.header.type = TELEM_POWER_DATA;
  power_telem.header.timestamp = xTaskGetTickCount();
  power_telem.header.sequence = sequence_number++;
  power_telem.header.priority = 2;

  // Voltaje de batería: 3.3V ± 0.05V (variación típica de Li-Ion)
  float voltage_variation = ((esp_random() % 100) - 50) / 1000.0f; // -0.05 a +0.05
  power_telem.battery_voltage = 3.3f + voltage_variation;
  
  // Temperatura de batería: 25°C ± 3°C
  int8_t temp_variation = (esp_random() % 7) - 3; // -3 a +3
  power_telem.battery_temperature = 25 + temp_variation;
  
  // Corriente de batería: 0.1A ± 0.02A
  float current_variation = ((esp_random() % 40) - 20) / 1000.0f; // -0.02 a +0.02
  power_telem.battery_current = 0.1f + current_variation;
  
  // Panel solar: 5.0V ± 0.1V (depende de iluminación)
  float solar_v_variation = ((esp_random() % 200) - 100) / 1000.0f; // -0.1 a +0.1
  power_telem.solar_panel_voltage = 5.0f + solar_v_variation;
  
  // Corriente solar: 0.5A ± 0.1A
  float solar_i_variation = ((esp_random() % 200) - 100) / 1000.0f; // -0.1 a +0.1
  power_telem.solar_panel_current = 0.5f + solar_i_variation;
  
  // Degradación lenta de batería: 1% cada 15 minutos real.
  uint32_t uptime_sec = (uint32_t)(xTaskGetTickCount() / configTICK_RATE_HZ);
  uint8_t level = 85;
  if (uptime_sec >= 900) {
    uint32_t drop = uptime_sec / 900; // cada 15 minutos baja 1%
    level = (drop >= 85) ? 0 : (uint8_t)(85 - drop);
  }
  power_telem.battery_level = level;
  power_telem.power_state = 0;

  telemetry_store_packet((telemetry_packet_t*)&power_telem);
}


void generate_temperature_telemetry(void) {
  temperature_telem_t temp_telem;

  temp_telem.header.type = TELEM_TEMPERATURE_DATA;
  temp_telem.header.timestamp = xTaskGetTickCount();
  temp_telem.header.sequence = sequence_number++;
  temp_telem.header.priority = 1;

  // OBC: 35°C ± 2°C (procesador trabaja con carga variable)
  temp_telem.obc_temperature = 35 + ((esp_random() % 5) - 2);
  
  // Comms: 28°C ± 2°C (transmisor puede calentarse)
  temp_telem.comms_temperature = 28 + ((esp_random() % 5) - 2);
  
  // Payload: 25°C ± 1°C (usualmente más estable)
  temp_telem.payload_temperature = 25 + ((esp_random() % 3) - 1);
  
  // Batería: 22°C ± 2°C (reacción exotérmica en carga/descarga)
  temp_telem.battery_temperature = 22 + ((esp_random() % 5) - 2);
  
  // Exterior: -15°C ± 5°C (exposición solar variable en órbita)
  temp_telem.external_temperature = -15 + ((esp_random() % 11) - 5);

  telemetry_store_packet((telemetry_packet_t*)&temp_telem);
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
  uint32_t uptime_sec2 = (uint32_t)(xTaskGetTickCount() / configTICK_RATE_HZ);
  subsys_telem.comms_uptime = uptime_sec2;
  subsys_telem.payload_uptime = (uptime_sec2 > 100) ? (uptime_sec2 - 100) : 0;
  subsys_telem.last_command_id = 0x25;
  
  // Success rate: 98% ± 2% (simular pequeñas fluctuaciones por ruido)
  int variation = (esp_random() % 5) - 2; // -2 a +2
  int success_rate = 98 + variation;
  subsys_telem.command_success_rate = (success_rate < 0) ? 0 : ((success_rate > 100) ? 100 : (uint8_t)success_rate);

  telemetry_store_packet((telemetry_packet_t*)&subsys_telem);
}