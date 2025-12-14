/**
 * @file telemetry_types.h
 * @brief Definiciones de tipos de telemetría y paquetes para el sistema
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 30-10-2025
 *
 * @details
 * Tipos y estructuras usadas por el sistema de telemetría. Incluye el
 * encabezado común `telem_header_t`, los distintos bloques de datos de
 * telemetría (sistema, potencia, temperatura, subsistemas) y la unión
 * `telemetry_packet_t` para almacenar cualquier paquete.
 * 
 * @see https://www.luisllamas.es/en/esp32-built-in-temperature-sensor/
 */

#ifndef TELEMETRY_TYPES_H
#define TELEMETRY_TYPES_H

#include <stdint.h>
#include <time.h>

/** @brief Tipos de datos de telemetría disponibles */
typedef enum {
    TELEM_SYSTEM_STATUS = 0,      /**< Estado general del sistema */
    TELEM_POWER_DATA,             /**< Datos del sistema de potencia */
    TELEM_TEMPERATURE_DATA,       /**< Mediciones de temperatura */
    TELEM_COMMUNICATION_STATUS    /**< Estado de comunicaciones */
} telem_data_type_t;

/**
 * @brief Encabezado común para todos los paquetes de telemetría
 *
 * @details Contiene el tipo de dato, timestamp, número de secuencia y
 * prioridad. Se incluye como primer miembro en todas las estructuras de
 * telemetría para permitir un manejo genérico de paquetes.
 */
typedef struct {
    telem_data_type_t type;   /**< Tipo de telemetría (ver telem_data_type_t) */
    uint32_t timestamp;       /**< Timestamp interno del sistema (segundos) */
    uint16_t sequence;        /**< Número de secuencia del paquete */
    uint8_t priority;         /**< Prioridad (0=low,1=normal,2=high) */
} telem_header_t;

/**
 * @brief Datos de estado general del sistema
 *
 * @details Contiene métricas del OBC como uptime, uso CPU, marcas de pila y memoria
 * libre, así como conteos de tareas y errores.
 */
typedef struct {
    telem_header_t header;        /**< Encabezado común */
    uint32_t uptime_seconds;      /**< Segundos desde arranque */
    uint8_t system_mode;          /**< 0=safe,1=nominal,2=payload,3=critical */
    uint8_t cpu_usage;            /**< Uso de CPU en porcentaje 0-100 */
    uint16_t stack_high_water;    /**< FreeRTOS stack high water mark */
    uint32_t heap_free;           /**< Memoria heap libre en bytes */
    uint8_t task_count;           /**< Número de tareas activas */
    float cpu_temperature;       /**< Temperatura CPU del ESP32 en celsius */
} system_status_telem_t;

/**
 * @brief Datos del sistema de potencia
 *
 * @details Incluye voltajes y corrientes de batería y paneles solares, nivel y
 * temperatura de la batería, y estado general del subsistema de potencia.
 */
typedef struct {
    telem_header_t header;        /**< Encabezado común */
    float battery_voltage;        /**< Voltaje de batería (V) */
    float battery_current;        /**< Corriente de batería (A) */
    float solar_panel_voltage;    /**< Voltaje de paneles solares (V) */
    float solar_panel_current;    /**< Corriente de paneles solares (A) */
    uint8_t battery_level;        /**< Nivel de batería 0-100% */
    int8_t battery_temperature;   /**< Temperatura batería (°C) */
    uint8_t power_state;          /**< Estado de potencia (codificado) */
} power_telem_t;

/**
 * @brief Datos de temperatura del bus espacial
 *
 * @details Temperaturas reportadas por distintos sensores a bordo.
 */
typedef struct {
    telem_header_t header;          /**< Encabezado común */
    int16_t obc_temperature;        /**< Temperatura OBC (0.1°C o °C según convención) */
    int16_t comms_temperature;      /**< Temperatura módulo comunicaciones */
    int16_t payload_temperature;    /**< Temperatura del payload */
    int16_t battery_temperature;    /**< Temperatura de la batería */
    int16_t external_temperature;   /**< Temperatura externa/ambiente */
} temperature_telem_t;

/**
 * @brief Estado de subsistemas / máquinas a bordo
 *
 * @details Proporciona estados operativos y métricas de uptime por subsistema,
 * así como información del último comando ejecutado.
 */
typedef struct {
    telem_header_t header;          /**< Encabezado común */
    uint8_t comms_status;           /**< Estado comunicaciones (codificado) */
    uint8_t adcs_status;            /**< Estado ADCS/control de actitud (No probado) */
    uint8_t payload_status;         /**< Estado del payload (No probado) */
    uint8_t power_status;           /**< Estado del sistema de potencia */
    uint32_t comms_uptime;          /**< Tiempo activo comunicaciones (s) */
    uint32_t payload_uptime;        /**< Tiempo activo payload (s) */
    uint8_t last_command_id;        /**< ID del último comando ejecutado */
    uint8_t command_success_rate;   /**< Tasa de éxito de comandos (%) */
} subsystem_status_telem_t;

/**
 * @brief Unión que representa un paquete de telemetría genérico
 *
 * @details permite almacenar cualquier tipo de telemetría en un mismo contenedor.
 * `raw_data` proporciona un espacio para datos genéricos o futuros formatos.
 */
typedef union {
    telem_header_t header;                 /**< Acceso al encabezado común */
    system_status_telem_t system;          /**< Datos de estado del sistema */
    power_telem_t power;                   /**< Datos de potencia */
    temperature_telem_t temperature;       /**< Datos de temperatura */
    subsystem_status_telem_t subsystems;   /**< Estados de subsistemas */
    uint8_t raw_data[64];                  /**< Buffer crudo para datos genéricos */
} telemetry_packet_t;

#endif // TELEMETRY_TYPES_H