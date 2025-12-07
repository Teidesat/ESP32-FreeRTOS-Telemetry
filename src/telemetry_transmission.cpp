/**
 * @file telemetry_transmission.cpp
 * @brief Implementación del módulo de transmisión de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 22-11-2025
 *
 * @details
 * Este módulo se encarga de transmitir los paquetes de telemetría almacenados,
 * simulando la comunicación con la estación terrestre.
 * Envía datos en formato JSON compatible con Fomalhaut.
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "../include/telemetry_transmission.h"
#include "../include/telemetry_storage.h"
#include "../include/telemetry_logger.h"

static uint32_t s_transmitted_total = 0;
static bool s_ground_window_open = false;
static TickType_t s_last_window_tick = 0;

void telemetry_transmission_init(void) {
  telemetry_logf("[XMIT] Init OK - JSON mode enabled");
  s_last_window_tick = xTaskGetTickCount();
}

/**
 * @brief Envía un paquete de telemetría en formato JSON por Serial
 * @param packet Paquete de telemetría a enviar
 */
static void send_json_packet(const telemetry_packet_t* packet) {
  if (!packet) return;
  
  switch(packet->header.type) {
    case TELEM_SYSTEM_STATUS: {
      const system_status_telem_t* sys = &packet->system;
      Serial.print("{\"type\":\"system\",\"cpuUsage\":");
      Serial.print(sys->cpu_usage);
      Serial.print(",\"memoryFree\":");
      Serial.print(sys->heap_free);
      Serial.print(",\"uptime\":");
      Serial.print(sys->uptime_seconds);
      Serial.print(",\"taskCount\":");
      Serial.print(sys->task_count);
      Serial.print(",\"cpuTemp\":");
      Serial.print(sys->cpu_temperature, 1);
      Serial.println("}");
      break;
    }
    
    case TELEM_POWER_DATA: {
      const power_telem_t* pwr = &packet->power;
      Serial.print("{\"type\":\"power\",\"voltage\":");
      Serial.print(pwr->battery_voltage, 2);
      Serial.print(",\"current\":");
      Serial.print(pwr->battery_current, 3);
      Serial.print(",\"solarVoltage\":");
      Serial.print(pwr->solar_panel_voltage, 2);
      Serial.print(",\"solarCurrent\":");
      Serial.print(pwr->solar_panel_current, 3);
      Serial.print(",\"batteryLevel\":");
      Serial.print(pwr->battery_level);
      Serial.print(",\"batteryTemp\":");
      Serial.print(pwr->battery_temperature);
      Serial.println("}");
      break;
    }
    
    case TELEM_TEMPERATURE_DATA: {
      // Las temperaturas se envían directamente desde vTelemetryGeneratorTask
      // No se transmiten desde storage para evitar problemas de serialización
      // Simplemente saltamos este tipo
      break;
    }
    
    case TELEM_COMMUNICATION_STATUS: {
      const subsystem_status_telem_t* sub = &packet->subsystems;
      // RSSI: -50 a -80 dBm (como en main.cpp original)
      int rssi = -50 - (esp_random() % 31);
      // SNR: 8-18 dB (como en main.cpp original)
      int snr = 8 + (esp_random() % 11);
      Serial.print("{\"type\":\"comms\",\"rssi\":");
      Serial.print(rssi);
      Serial.print(",\"snr\":");
      Serial.print(snr);
      Serial.print(",\"commsUptime\":");
      Serial.print(sub->comms_uptime);
      Serial.print(",\"successRate\":");
      Serial.print(sub->command_success_rate);
      Serial.println("}");
      break;
    }
  }
}

static void open_window(void) {
  s_ground_window_open = true;
  telemetry_logf("\n🎯 GROUND STATION CONTACT WINDOW OPEN!");
}

void telemetry_transmission_cycle(void) {
  // NUEVO: Transmitir continuamente sin esperar ventanas de contacto (para desarrollo)
  uint32_t available = telemetry_available_packets();
  
  if(available == 0) {
    return; // Nada que hacer
  }

  telemetry_logf("📤 TRANSMITTING %lu packets...", available);
  telemetry_packet_t packet;
  static uint32_t s_transmitted_total = 0;
  
  while(telemetry_retrieve_packet(&packet)) {
    s_transmitted_total++;
    
    // Enviar en formato JSON para Fomalhaut
    send_json_packet(&packet);
    
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  telemetry_logf("✅ Transmission complete. Total sent: %lu packets", s_transmitted_total);
}
