/**
 * @file telemetry_processing.cpp
 * @brief Implementación del módulo de procesamiento de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 22-11-2025
 *
 * @details
 * Este módulo se encarga de procesar los paquetes de telemetría recibidos,
 * interpretando sus datos y generando logs informativos.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../include/telemetry_processing.h"
#include "../include/telemetry_storage.h"
#include "../include/telemetry_logger.h"

void telemetry_processing_init(void) {
  telemetry_logf("[PROC] Init OK");
}

bool telemetry_processing_handle_one(void) {
  telemetry_packet_t packet;
  if(!telemetry_retrieve_packet(&packet)) {
    return false;
  }

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
    default:
      telemetry_logf("[PROC] Unknown packet type=%d seq=%d", packet.header.type, packet.header.sequence);
    break;
    }

  telemetry_logf("   Available packets: %lu", telemetry_available_packets());
  return true;
}
