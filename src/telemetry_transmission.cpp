/**
 * @file telemetry_transmission.cpp
 * @brief Implementación del módulo de transmisión de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 22-11-2025
 *
 * @details
 * Este módulo se encarga de transmitir los paquetes de telemetría almacenados,
 * simulando la comunicación con la estación terrestre.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../include/telemetry_transmission.h"
#include "../include/telemetry_storage.h"
#include "../include/telemetry_logger.h"

static uint32_t s_transmitted_total = 0;
static bool s_ground_window_open = false;
static TickType_t s_last_window_tick = 0;

void telemetry_transmission_init(void) {
  telemetry_logf("[XMIT] Init OK");
  s_last_window_tick = xTaskGetTickCount();
}

static void open_window(void) {
  s_ground_window_open = true;
  telemetry_logf("\n🎯 GROUND STATION CONTACT WINDOW OPEN!");
}

void telemetry_transmission_cycle(void) {
  TickType_t now = xTaskGetTickCount();
  // Abrir ventana cada ~30 segundos virtuales (ajustable)
  if(((now / 1000) % 30) == 0 && (now - s_last_window_tick) > pdMS_TO_TICKS(500)) {
    open_window();
    s_last_window_tick = now;
  }

  if(!s_ground_window_open) {
    return; // Nada que hacer aún
  }

  uint32_t available = telemetry_available_packets();
  if(available == 0) {
    // Cerrar la ventana si no hay datos
    s_ground_window_open = false;
    return;
  }

  telemetry_logf("📤 TRANSMITTING %lu packets to ground...", available);
  telemetry_packet_t packet;
  while(telemetry_retrieve_packet(&packet)) {
    s_transmitted_total++;
    telemetry_logf("   📦 [%lu] Type=%d, Seq=%d, Time=%lu", s_transmitted_total,
      packet.header.type, packet.header.sequence, packet.header.timestamp);
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  telemetry_logf("✅ Transmission complete. Total sent: %lu packets", s_transmitted_total);
  s_ground_window_open = false;
}
