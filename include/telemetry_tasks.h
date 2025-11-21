/**
 * @file telemetry_tasks.h
 * @brief Declaraciones de tareas FreeRTOS del sistema de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 20-10-2025
 * 
 * @details
 * Este archivo contiene las declaraciones de las tareas FreeRTOS que componen
 * el sistema de telemetría del satélite TeideSat.
 * 
 * El sistema está compuesto por tres tareas principales que ejecutan
 * concurrentemente:
 * - Recolector: Genera y almacena datos de telemetría
 * - Procesador: Procesa y visualiza los datos almacenados
 * - Transmisor: Simula el envío de datos a estación terrestre
 * 
 * @note Las tareas están optimizadas para entorno WOKWI con intervalos
 * reducidos para facilitar la visualización durante pruebas.
 */

#ifndef TELEMETRY_TASKS_H
#define TELEMETRY_TASKS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Tarea recolectora de datos de telemetría
 * @param pvParameters Parámetros de la tarea (no utilizados en esta implementación)
 * 
 * @details
 * Esta tarea es responsable de la generación periódica de todos los tipos
 * de telemetría del satélite. Se ejecuta cada 5 segundos y genera datos de:
 * - Estado del sistema (uptime, memoria, tareas)
 * - Sistema de potencia (voltaje, corriente, batería)
 * - Temperaturas de todos los subsistemas
 * - Estado operativo de subsistemas
 * 
 * La tarea utiliza la función vTaskDelayUntil() para mantener una periodicidad
 * precisa de 5 segundos, independiente del tiempo de ejecución de las funciones
 * generadoras.
 * 
 * @note En entorno de producción, los intervalos deberían ajustarse según
 * los requisitos específicos del proyecto y las limitaciones de energía.
 * 
 */
void vTelemetryCollectorTask(void *pvParameters);

/**
 * @brief Tarea procesadora de datos de telemetría
 * @param pvParameters Parámetros de la tarea (no utilizados en esta implementación)
 * 
 * @details
 * Esta tarea se encarga de recuperar los paquetes de telemetría del buffer
 * circular y procesarlos para su visualización y análisis. Las principales
 * funciones incluyen:
 * 
 * - Recuperación de paquetes del buffer de almacenamiento
 * - Procesamiento y formateo de datos para visualización
 * - Presentación estructurada en terminal
 * - Monitoreo del estado del buffer (paquetes disponibles)
 * 
 * La tarea implementa un patrón de consumo activo, donde verifica
 * constantemente la disponibilidad de nuevos paquetes. Cuando no hay datos
 * disponibles, entra en modo de espera para reducir el consumo de CPU.
 * En un sistema real, esta tarea podría incluir operaciones más
 * complejas como compresión, cifrado o detección de anomalías.
 * 
 */
void vTelemetryProcessorTask(void *pvParameters);

/**
 * @brief Tarea transmisora de datos de telemetría
 * @param pvParameters Parámetros de la tarea (no utilizados en esta implementación)
 * 
 * @details
 * Esta tarea simula el proceso de transmisión de telemetría a la estación
 * terrestre. Implementa un modelo de ventanas de comunicación típico en
 * satélites, donde la transmisión solo es posible durante periodos específicos
 * cuando el satélite está sobre una estación terrestre.
 * 
 * Características principales:
 * - Simula ventanas de comunicación cada ~30 segundos
 * - Transmite paquetes en lotes cuando hay conectividad
 * - Implementa un mecanismo de transmisión con confirmación visual
 * - Incluye pausas entre paquetes para simular latencia de transmisión
 * 
 * @note En un sistema real, esta tarea incluiría protocolos de comunicación
 * específicos (AX.25, CSP, etc.) y manejo de errores de transmisión.
 * @note La simulación de ventanas de comunicación utiliza una condición
 * temporal simple. En un satélite real, esto se basaría en efemérides y
 * posición orbital.
 */
void vTelemetryTransmitterTask(void *pvParameters);

#endif /* TELEMETRY_TASKS_H */