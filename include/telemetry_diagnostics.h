/**
 * @file telemetry_diagnostics.h
 * @brief Declaración del módulo de diagnóstico y salud del sistema de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 22-11-2025
 *
 * @details
 * Proporciona funciones para el diagnóstico periódico del sistema de telemetría,
 * incluyendo volcados de logs y reportes de estado del sistema.
 */

#ifndef TELEMETRY_DIAGNOSTICS_H
#define TELEMETRY_DIAGNOSTICS_H

/**
 * @brief Inicializa el módulo de diagnóstico
 * 
 * @details
 * Configura los temporizadores internos y prepara el sistema para
 * realizar diagnósticos periódicos.
 */
void telemetry_diagnostics_init(void);

/**
 * @brief Función de tick para diagnóstico periódico
 * 
 * @details
 * Debe llamarse periódicamente (ej. desde el loop principal) para
 * ejecutar las tareas de diagnóstico, como volcados de logs y
 * reportes de estado del sistema.
 */
void telemetry_diagnostics_tick(void);

#endif /* TELEMETRY_DIAGNOSTICS_H */
