/**
 * @file telemetry_generators.h
 * @brief Declaraciones de funciones generadoras de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 20-10-2025
 * 
 * @details
 * Este archivo contiene las declaraciones de las funciones que generan
 * los diferentes tipos de datos de telemetría del satélite.
 */

#ifndef TELEMETRY_GENERATORS_H
#define TELEMETRY_GENERATORS_H

/**
 * @brief Genera datos de telemetría del estado del sistema
 * 
 * @details
 * Recopila y genera información sobre el estado general del sistema:
 * - Tiempo de actividad (uptime)
 * - Modo de operación del sistema
 * - Uso de memoria heap disponible
 * - Número de tareas activas en FreeRTOS
 * 
 * @note Los datos de uso de CPU no están disponibles en ESP32 FreeRTOS
 */
void generate_system_telemetry(void);

/**
 * @brief Genera datos de telemetría del sistema de potencia
 * 
 * @details
 * Recopila información sobre el estado del sistema de potencia:
 * - Voltaje y corriente de batería
 * - Voltaje y corriente de paneles solares  
 * - Nivel de carga de la batería
 * - Temperatura de la batería
 * - Estado general del sistema de potencia
 * 
 * Esta telemetría tiene prioridad alta debido a su importancia crítica
 * 
 * @note El nivel de batería disminuye gradualmente con el tiempo
 * para simular el consumo energético real.
 */
void generate_power_telemetry(void);

/**
 * @brief Genera datos de telemetría de temperaturas
 * 
 * @details
 * Recopila temperaturas de los diferentes componentes del satélite:
 * - OBC (On-Board Computer)
 * - Módulo de comunicaciones  
 * - Payload (carga útil)
 * - Batería
 * - Temperatura externa
 * 
 * Las temperaturas se generan para cada componente,
 * simulando los gradientes térmicos típicos en un satélite.
 * 
 * @note En un sistema real, estos datos vendrían de sensores de temperatura
 * como termistores o sensores I2C (DS18B20, TMP36, etc.).
 */
void generate_temperature_telemetry(void);

/**
 * @brief Genera datos de telemetría del estado de subsistemas
 * 
 * @details
 * Proporciona información sobre el estado operativo de los principales
 * subsistemas del satélite:
 * - Estado de comunicaciones (operacional/fallo)
 * - Estado del ADCS 
 * - Estado del payload 
 * - Estado del sistema de potencia
 * - Tiempos de actividad de subsistemas
 * - Estadísticas de ejecución de comandoss
 */
void generate_subsystem_telemetry(void);

#endif /* TELEMETRY_GENERATORS_H */