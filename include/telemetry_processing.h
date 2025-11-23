/**
 * @file telemetry_processing.h
 * @brief Declaración del módulo de procesado y formateo de paquetes de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 22-11-2025
 *
 * @details
 * Este módulo se encarga de inicializar el sistema de procesamiento de telemetría
 * y de manejar la recuperación y el procesamiento de paquetes de telemetría.
 */

#ifndef TELEMETRY_PROCESSING_H
#define TELEMETRY_PROCESSING_H

#include <stdbool.h>
#include "telemetry_types.h"

/**
 * @brief Inicializa el módulo de procesamiento de telemetría
 * 
 * @details
 * Configura los recursos necesarios para el procesamiento de paquetes de telemetría.
 */
void telemetry_processing_init(void);

/**
 * @brief Intenta recuperar y procesar un paquete de telemetría
 * 
 * @details
 * Esta función intenta recuperar un paquete de telemetría del buffer
 * y procesarlo según su tipo. Devuelve true si se procesó un paquete,
 * o false si no había paquetes disponibles.
 * 
 * @return true Si se procesó un paquete
 * @return false Si no había paquetes disponibles
 */
bool telemetry_processing_handle_one(void);

#endif /* TELEMETRY_PROCESSING_H */
