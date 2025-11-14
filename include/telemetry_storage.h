/**
 * @file telemetry_storage.h
 * @brief Sistema de almacenamiento de telemetría con buffer circular para FreeRTOS
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 20-10-2025
 *
 * @details
 * Este módulo declara un sistema de almacenamiento de telemetría basado en un
 * buffer circular con sincronización mediante mutex de FreeRTOS. Diseñado
 * específicamente para ejecutarse en el ESP32 bajo FreeRTOS.
 *
 * Características principales:
 * - Buffer circular de tamaño fijo optimizado para ESP32
 * - Protección multitarea mediante mutex de FreeRTOS
 * - Manejo eficiente de condiciones de buffer lleno
 * - Estadísticas de uso y pérdida de paquetes
 * - Timeout configurable para operaciones de mutex
 * 
 * @see https://github.com/CDFER/Ring-Buffer-Demo-ESP32-Arduino
 * @see https://www.youtube.com/watch?v=09HHWATPcwY
 */

#ifndef TELEMETRY_STORAGE_H
#define TELEMETRY_STORAGE_H

#include "telemetry_types.h"
#include <stdbool.h>
#include "FS.h"
#include "LittleFS.h"

/* ESP32 FreeRTOS includes */
#if ESP_PLATFORM
  #include "freertos/FreeRTOS.h"
  #include "freertos/semphr.h"
  #include "freertos/task.h"
#else
  #error "Este código requiere ESP32 FreeRTOS"
#endif

/** @brief Capacidad máxima del buffer circular en número de paquetes */
#define TELEM_BUFFER_SIZE 1024

/** @brief Tamaño en bytes de cada paquete de telemetría */
#define TELEM_PACKET_SIZE sizeof(telemetry_packet_t)

/**
 * @brief Estructura principal del buffer circular de telemetría
 *
 * @details Esta estructura mantiene el estado completo del buffer circular,
 * incluyendo los datos almacenados, índices de lectura/escritura, contadores
 * estadísticos y el mutex para sincronización.
 */
typedef struct {
  telemetry_packet_t buffer[TELEM_BUFFER_SIZE];  /**< Array circular de paquetes */
  uint32_t write_index;                          /**< Índice de escritura actual */
  uint32_t read_index;                           /**< Índice de lectura actual */
  uint32_t packets_written;                      /**< Total de paquetes escritos */
	uint32_t packets_read;                         /**< Total de paquetes leídos */
  uint32_t packets_lost;                         /**< Paquetes perdidos por buffer lleno */
  SemaphoreHandle_t mutex;                       /**< Mutex para sincronización */
} telemetry_buffer_t;

/**
 * @brief Inicializa el sistema de almacenamiento de telemetría
 * 
 * @details Inicializa el buffer circular y crea el mutex de sincronización.
 * Debe llamarse antes de usar cualquier otra función del módulo.
 */
void telemetry_storage_init(void);

/**
 * @brief Almacena un nuevo paquete de telemetría en el buffer
 * 
 * @param packet Puntero al paquete de telemetría a almacenar
 * @return true Si el paquete se almacenó correctamente
 * @return false Si el buffer está lleno o hay error de sincronización
 * 
 * @note Esta función es segura y puede ser llamada desde cualquier tarea
 */
bool telemetry_store_packet(const telemetry_packet_t* packet);

/**
 * @brief Recupera el siguiente paquete disponible del buffer
 * 
 * @param packet Puntero donde se almacenará el paquete leído
 * @return true Si se recuperó un paquete correctamente
 * @return false Si el buffer está vacío o hay error de sincronización
 * 
 * @note Esta función es segura y puede ser llamada desde cualquier tarea
 */
bool telemetry_retrieve_packet(telemetry_packet_t* packet);

/**
 * @brief Obtiene el número de paquetes disponibles para lectura
 * 
 * @return uint32_t Número de paquetes almacenados en el buffer
 */
uint32_t telemetry_available_packets(void);

/**
 * @brief Obtiene el espacio libre en el buffer
 * 
 * @return uint32_t Número de paquetes adicionales que pueden almacenarse
 */
uint32_t telemetry_free_space(void);

/**
 * @brief Obtiene estadísticas de uso del buffer
 * 
 * @param[out] written Puntero donde se almacenará el total de paquetes escritos
 * @param[out] read Puntero donde se almacenará el total de paquetes leídos
 * @param[out] lost Puntero donde se almacenará el total de paquetes perdidos
 * 
 * @note Los paquetes se consideran perdidos cuando el buffer está lleno y
 * no se pueden almacenar nuevos datos
 */
void telemetry_get_stats(uint32_t* written, uint32_t* read, uint32_t* lost);

#endif // TELEMETRY_STORAGE_H