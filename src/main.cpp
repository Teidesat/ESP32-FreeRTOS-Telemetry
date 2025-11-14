/**
 * @file main.cpp
 * @brief Sistema de Telemetría para Satélite TeideSat - Programa Principal
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 20-10-2025
 * 
 * @details
 * Este archivo contiene la implementación principal del sistema de telemetría
 * para el satélite TeideSat. El sistema está diseñado para ejecutarse en un
 * ESP32 y utiliza FreeRTOS para la gestión de tareas concurrentes.
 * 
 * El sistema implementa tres tareas principales:
 * - Recolector de telemetría: Captura datos de los sensores
 * - Procesador de telemetría: Procesa y analiza los datos capturados
 * - Transmisor de telemetría: Envía los datos procesados
 * 
 * @note Este código está optimizado para ejecutarse en un entorno Wokwi ESP32.
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "../include/telemetry_storage.h"

/*
 * DATO IMPORTANTE
 * El archivo main.cpp está en C++, pero las funciones de las tareas están implementadas en archivos .c (C puro)
 * extern "C" le dice al compilador de C++ que use el sistema de nombres de C para estas funciones.
 * Sin esto, el linker no podría encontrar las funciones porque C++ modifica los nombres de las funciones para 
 * soportar sobrecarga, mientras que C no lo hace.
 */
extern "C" {
    /**
     * @brief Tarea para recolección de datos de telemetría
     * @param pvParameters Parámetros de la tarea (no utilizados)
     * 
     * @details Esta tarea se encarga de recopilar datos de los diferentes sensores
     * y sistemas del satélite, incluyendo:
     * - Estado del sistema
     * - Datos de energía
     * - Temperaturas
     * - Estado de subsistemas
     * 
     * La tarea se ejecuta periódicamente y almacena los datos en el buffer circular
     * de telemetría para su posterior procesamiento.
     */
    void vTelemetryCollectorTask(void *pvParameters);

    /**
     * @brief Tarea para procesamiento de datos de telemetría
     * @param pvParameters Parámetros de la tarea (no utilizados)
     * 
     * @details Esta tarea procesa los datos almacenados en el buffer circular,
     * realizando operaciones como:
     * - Validación de datos
     * - Cálculo de estadísticas
     * - Detección de anomalías
     * - Preparación para transmisión
     */
    void vTelemetryProcessorTask(void *pvParameters);

    /**
     * @brief Tarea para transmisión de datos de telemetría
     * @param pvParameters Parámetros de la tarea (no utilizados)
     * 
     * @details Esta tarea se encarga de transmitir los datos procesados,
     * implementando:
     * - Priorización de mensajes
     * - Gestión de cola de transmisión
     * - Control de errores
     * - Confirmación de envío
     */
    void vTelemetryTransmitterTask(void *pvParameters);
}

/**
 * @brief Función de inicialización del sistema
 * 
 * @details Esta función se ejecuta una vez al inicio y realiza:
 * - Inicialización de la comunicación serial
 * - Creación de las tareas de FreeRTOS
 * - Configuración inicial del sistema
 * 
 * Las tareas se crean con diferentes prioridades:
 * - Recolector: Prioridad 2 (alta)
 * - Procesador: Prioridad 1 (normal)
 * - Transmisor: Prioridad 1 (normal)
 */
void setup() {
  Serial.begin(115200);

  // Esperar un poco que Serial esté listo
  delay(1000);

  Serial.print("Temp onBoard ");
  // Serial.print(temp_celsius);
  Serial.println("°C");

  Serial.println("\n🛰️  TEIDESAT SATELLITE TELEMETRY SYSTEM - ESP32 WOKWI");
  Serial.println("======================================================");
  Serial.println("Starting FreeRTOS tasks...");

  // Crear tareas de telemetría
  xTaskCreate(
    vTelemetryCollectorTask,   // Función
    "TelemCollect",            // Nombre
    4096,                      // Stack size
    NULL,                      // Parámetros
    2,                         // Prioridad
    NULL                       // Handle
  );

  xTaskCreate(
    vTelemetryProcessorTask,
    "TelemProcess", 
    4096,
    NULL,
    1,
    NULL
  );

  xTaskCreate(
    vTelemetryTransmitterTask,
    "TelemXmit",
    4096,
    NULL,
    1,
    NULL
  );

  Serial.println("✅ All telemetry tasks created successfully");
  Serial.println("📡 System operational - Telemetry data generation started");
  Serial.println("--------------------------------------------------------\n");
}

/**
 * @brief Bucle principal del programa
 * 
 * @details En un sistema basado en FreeRTOS, el loop principal se utiliza
 * principalmente para tareas de mantenimiento y monitoreo. En este caso:
 * - Muestra estadísticas del sistema cada 30 segundos
 * - Monitorea el estado general del ESP32
 * - Reporta uso de memoria y número de tareas
 * 
 * @note La mayoría del trabajo real se realiza en las tareas de FreeRTOS,
 * no en este loop.
 */
void loop() {
  // FreeRTOS maneja las tareas, este loop puede estar vacío
  delay(1000);

	// Opcional: mostrar estado general periódicamente
  static uint32_t last_status = 0;
  if(millis() - last_status > 30000) { // Cada 30 segundos
    last_status = millis();
    Serial.printf("\n📈 SYSTEM STATUS: Uptime: %lus | Heap: %lu | Tasks: %d\n",
                    millis() / 1000, 
                    esp_get_free_heap_size(),
                    uxTaskGetNumberOfTasks());
  }
}