/**
 * @file telemetry_logger.h 
 * @brief Logger de telemetría usando LittleFS
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 10-11-2025
 * 
 * @details
 * Este módulo proporciona funciones para registrar datos de telemetría
 * en un sistema de archivos LittleFS, permitiendo almacenar, visualizar
 * y gestionar los registros de telemetría de manera eficiente.
 */

#ifndef TELEMETRY_LOGGER_H
#define TELEMETRY_LOGGER_H

#include <stdbool.h>

// Archivos de log separados por tipo de telemetría
#define TELEMETRY_LOG_FILE "/telemetry_log.txt"      // Log general (eventos del sistema)
#define TELEMETRY_SYSTEM_LOG "/telem_system.txt"     // Telemetría de sistema
#define TELEMETRY_POWER_LOG "/telem_power.txt"       // Telemetría de potencia
#define TELEMETRY_TEMP_LOG "/telem_temp.txt"         // Telemetría de temperatura
#define TELEMETRY_COMMS_LOG "/telem_comms.txt"       // Telemetría de comunicaciones

/**
 * @brief Inicializa el sistema de logging de telemetría
 *  
 * @details Inicializa el sistema de archivos LittleFS 
 * y prepara el archivo de log para escritura. 
 * 
 * @note Debe llamarse antes de usar cualquier otra función del módulo. 
 */
bool telemetry_logger_init(void);

/**
 * @brief Escribe una linea formateada al Serial y al archivo.
 * 
 * @param fmt Formato de cadena estilo printf. 
 * @param ... Argumentos variables según el formato.
 * 
 * @details Escribe una línea formateada tanto en el Serial 
 * como en el archivo de log. Esto permite mantener un registro 
 * persistente de los eventos y datos de telemetría para su posterior análisis.
 */
void telemetry_logf(const char *fmt, ...);

/**
 * @brief Vuelca el contenido completo del archivo de log por Serial.
 * 
 * @details Lee el archivo de log completo y lo imprime en el
 * puerto Serial. Esto es útil para depuración y análisis
 * de los datos registrados durante la operación del sistema.
 */
void telemetry_dump_log(void);

/**
 * @brief Borra (trunca) el archivo de log para empezar desde cero.
 * 
 * @details Esta función elimina todo el contenido del archivo de log,
 * permitiendo comenzar un nuevo registro desde cero. Es útil para
 * limpiar registros antiguos y liberar espacio en el sistema de archivos.
 */
void telemetry_log_clear(void);

/**
 * @brief Limpia todos los archivos de telemetría
 * 
 * @details Trunca todos los archivos de telemetría (sistema, potencia,
 * temperatura, comunicaciones) para comenzar con registros limpios.
 * Se llama automáticamente en telemetry_logger_init().
 */
void telemetry_clear_all_logs(void);

/**
 * @brief Escribe datos de telemetría de sistema en su archivo dedicado
 * @param fmt Formato printf
 * @param ... Argumentos variables
 */
void telemetry_log_system(const char *fmt, ...);

/**
 * @brief Escribe datos de telemetría de potencia en su archivo dedicado
 * @param fmt Formato printf
 * @param ... Argumentos variables
 */
void telemetry_log_power(const char *fmt, ...);

/**
 * @brief Escribe datos de telemetría de temperatura en su archivo dedicado
 * @param fmt Formato printf
 * @param ... Argumentos variables
 */
void telemetry_log_temperature(const char *fmt, ...);

/**
 * @brief Escribe datos de telemetría de comunicaciones en su archivo dedicado
 * @param fmt Formato printf
 * @param ... Argumentos variables
 */
void telemetry_log_comms(const char *fmt, ...);

/**
 * @brief Vuelca el contenido del archivo de sistema por Serial
 */
void telemetry_dump_system_log(void);

/**
 * @brief Vuelca el contenido del archivo de potencia por Serial
 */
void telemetry_dump_power_log(void);

/**
 * @brief Vuelca el contenido del archivo de temperatura por Serial
 */
void telemetry_dump_temperature_log(void);

/**
 * @brief Vuelca el contenido del archivo de comunicaciones por Serial
 */
void telemetry_dump_comms_log(void);

/**
 * @brief Vuelca todos los archivos de telemetría por Serial
 */
void telemetry_dump_all_logs(void);

#endif // TELEMETRY_LOGGER_H