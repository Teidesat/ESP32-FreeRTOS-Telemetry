/**
 * @file telemetry_transmission.h
 * @brief Declaración del módulo de transmisión de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 22-11-2025
 *
 * @details
 * Este módulo se encarga de inicializar y gestionar la transmisión de datos de telemetría.
 */

#ifndef TELEMETRY_TRANSMISSION_H
#define TELEMETRY_TRANSMISSION_H

#include <stdbool.h>
#include "telemetry_types.h"

/** 
 * @brief Inicializa el módulo de transmisión de telemetría
 * 
 * @details
 * Configura los recursos necesarios para la transmisión de datos
 * de telemetría, incluyendo la preparación de ventanas de contacto
 * con la estación terrestre.
 */
void telemetry_transmission_init(void);

/** 
 * @brief Ejecuta un ciclo de transmisión de telemetría
 * 
 * @details
 * Esta función debe ser llamada periódicamente para gestionar
 * la transmisión de paquetes de telemetría durante las ventanas
 * de contacto con la estación terrestre.
 */
void telemetry_transmission_cycle(void);

#endif /* TELEMETRY_TRANSMISSION_H */
