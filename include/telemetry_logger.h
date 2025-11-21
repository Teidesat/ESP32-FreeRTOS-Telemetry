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

#define TELEMETRY_LOG_FILE "/telemetry_log.txt"

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

#endif // TELEMETRY_LOGGER_H