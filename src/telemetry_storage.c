/**
 * @file telemetry_storage.c
 * @brief Implementación del buffer circular de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 20-10-2025
 * 
 * @details
 * Este archivo contiene la implementación del sistema de almacenamiento de
 * telemetría basado en un buffer circular con sincronización mediante mutex
 * de FreeRTOS. Diseñado específicamente para ejecutarse en el ESP32 bajo FreeRTOS.
 */

#include "../include/telemetry_storage.h"

#if ESP_PLATFORM
  #include "freertos/FreeRTOS.h"
  #include "freertos/semphr.h"
  #include "freertos/task.h"
#endif

/** @brief Instancia global del buffer circular (static para encapsulamiento) */
static telemetry_buffer_t telem_buffer;

void telemetry_storage_init(void) {
  /* Inicialización de índices y contadores */
  telem_buffer.write_index = 0;
  telem_buffer.read_index = 0;
  telem_buffer.packets_written = 0;
  telem_buffer.packets_read = 0;
  telem_buffer.packets_lost = 0;

  /* Crear e inicializar el mutex */
  telem_buffer.mutex = xSemaphoreCreateMutex();

  if (telem_buffer.mutex == NULL) {
    /* Error crítico: no se pudo crear el mutex */
    while(1) {
      /* En un sistema real, aquí deberíamos notificar el error */
      vTaskDelay(pdMS_TO_TICKS(1000)); // "The macro pdMS_TO_TICKS() can be used to convert milliseconds into ticks." 
    }
  }
}

bool telemetry_store_packet(const telemetry_packet_t* packet) {
  if(xSemaphoreTake(telem_buffer.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    // Verificar si hay espacio
    uint32_t next_write = (telem_buffer.write_index + 1) % TELEM_BUFFER_SIZE;

    if(next_write == telem_buffer.read_index) {
      // Buffer lleno
      telem_buffer.packets_lost++;
      xSemaphoreGive(telem_buffer.mutex);
      return false;
    }

    // Almacenar paquete
    telem_buffer.buffer[telem_buffer.write_index] = *packet;
    telem_buffer.write_index = next_write;
    telem_buffer.packets_written++;

    xSemaphoreGive(telem_buffer.mutex);
    return true;
  }
  return false;
}

bool telemetry_retrieve_packet(telemetry_packet_t* packet) {
  if(xSemaphoreTake(telem_buffer.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    if(telem_buffer.read_index == telem_buffer.write_index) {
      // Buffer vacío
      xSemaphoreGive(telem_buffer.mutex);
      return false;
    }

    // Recuperar paquete
    *packet = telem_buffer.buffer[telem_buffer.read_index];
    telem_buffer.read_index = (telem_buffer.read_index + 1) % TELEM_BUFFER_SIZE;
    telem_buffer.packets_read++;

    xSemaphoreGive(telem_buffer.mutex);
    return true;
  }
  return false;
}

uint32_t telemetry_available_packets(void) {
  uint32_t available = 0;
  if(xSemaphoreTake(telem_buffer.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    if(telem_buffer.write_index >= telem_buffer.read_index) {
      available = telem_buffer.write_index - telem_buffer.read_index;
    } else {
      available = TELEM_BUFFER_SIZE - telem_buffer.read_index + telem_buffer.write_index;
    }
    xSemaphoreGive(telem_buffer.mutex);
  }
  return available;
}