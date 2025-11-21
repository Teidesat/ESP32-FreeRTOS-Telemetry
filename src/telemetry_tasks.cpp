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


void vTelemetryCollectorTask(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();

  telemetry_storage_init();
  telemetry_logf("🚀 Telemetry Collector Task Started");

  for(;;) {
    generate_system_telemetry();
    generate_power_telemetry();
    generate_temperature_telemetry(); 
    generate_subsystem_telemetry();
    
    // En WOKWI podemos usar intervalos más cortos para ver datos rápido
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000)); // 5 segundos
  }
}


void vTelemetryProcessorTask(void *pvParameters) {
  telemetry_packet_t packet;
  uint32_t processed_count = 0;

  telemetry_logf("🔧 Telemetry Processor Task Started");

  for(;;) {
    if(telemetry_retrieve_packet(&packet)) {
      processed_count++;

      // Visualización
      switch(packet.header.type) {
        case TELEM_SYSTEM_STATUS:
          telemetry_logf("📊 SYSTEM: Uptime=%lus | Heap=%lu | Tasks=%d | CPU Temp=%.1fC | Seq=%d",
          packet.system.uptime_seconds,
          packet.system.heap_free,
          packet.system.task_count,
          packet.system.cpu_temperature,
          packet.header.sequence);
        break;

        case TELEM_POWER_DATA:
          telemetry_logf("🔋 POWER: Bat=%.2fV | Level=%d%% | Temp=%dC | Seq=%d", 
          packet.power.battery_voltage,
          packet.power.battery_level,
          packet.power.battery_temperature,
          packet.header.sequence);
        break;

        case TELEM_TEMPERATURE_DATA:
          telemetry_logf("🌡️ TEMP: OBC=%dC | COMMS=%dC | PAYLOAD=%dC | Seq=%d",
          packet.temperature.obc_temperature,
          packet.temperature.comms_temperature,
          packet.temperature.payload_temperature, 
          packet.header.sequence);
        break;

        case TELEM_COMMUNICATION_STATUS:
          telemetry_logf("📡 COMMS: Status=%d | Uptime=%lu | Success=%d%% | Seq=%d",
					packet.subsystems.comms_status,
          packet.subsystems.comms_uptime,
          packet.subsystems.command_success_rate,
          packet.header.sequence);
        break;
      }

  telemetry_logf("   Available packets: %lu", telemetry_available_packets());

		} else {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

void vTelemetryTransmitterTask(void *pvParameters) {
  telemetry_packet_t packet;
  bool ground_station_available = false;
  uint32_t transmission_count = 0;

  telemetry_logf("📡 Telemetry Transmitter Task Started");

  for(;;) {
    // Simular disponibilidad aleatoria de estación terrestre
    if((xTaskGetTickCount() / 1000) % 30 == 0) { // Cada ~30 segundos
      ground_station_available = true;
      telemetry_logf("\n🎯 GROUND STATION CONTACT WINDOW OPEN!");
    }

    if(ground_station_available) {
      uint32_t available = telemetry_available_packets();

      if(available > 0) {
        telemetry_logf("📤 TRANSMITTING %lu packets to ground...", available);

        while(telemetry_retrieve_packet(&packet)) {
          transmission_count++;
          telemetry_logf("   📦 [%lu] Type=%d, Seq=%d, Time=%lu",
          transmission_count, packet.header.type,
          packet.header.sequence, packet.header.timestamp);

          // Pequeña pausa para simular transmisión
          vTaskDelay(pdMS_TO_TICKS(50));
        }

        telemetry_logf("✅ Transmission complete. Total sent: %lu packets", transmission_count);
      }

      ground_station_available = false;
    }

    vTaskDelay(pdMS_TO_TICKS(2000)); // Revisar cada 2 segundos
  }
}