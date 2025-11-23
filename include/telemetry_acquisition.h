/**
 * @file telemetry_acquisition.h
 * @brief Declaración del módulo de adquisición de telemetría (wrapper de generadores)
 * @author TeideSat
 * @date 22-11-2025
 *
 * @details
 * Proporciona la interfaz pública para la fase de adquisición de datos
 * del pipeline de telemetría. Envuelve las funciones generadoras y define
 * punto único de inicialización y ciclo periódico.
 */
#ifndef TELEMETRY_ACQUISITION_H
#define TELEMETRY_ACQUISITION_H

/**
 * @brief Inicializa recursos necesarios para adquisición (almacenamiento, etc.)
 * 
 * @details
 * Esta función debe llamarse una vez al inicio del sistema para preparar
 * los recursos necesarios para la adquisición de telemetría, incluyendo
 * la inicialización del sistema de almacenamiento. 
 */
void telemetry_acquisition_init(void);

/**
 * @brief Ejecuta un ciclo de adquisición de telemetría
 * 
 * @details
 * Esta función genera todos los tipos de datos de telemetría
 * y los almacena en el buffer correspondiente. Debe llamarse
 * periódicamente según el intervalo deseado de adquisición.
 */
void telemetry_acquisition_cycle(void);

#endif /* TELEMETRY_ACQUISITION_H */
