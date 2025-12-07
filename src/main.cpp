/**
 * @file main.cpp
 * @brief Sistema de Telemetría Integrado para Fomalhaut
 * 
 * Integra todo el código modular (generators, storage, tasks, transmission)
 * para enviar datos JSON por Serial cada 3 segundos.
 * 
 * El output es EXACTAMENTE igual al anterior:
 * - 4 tipos de JSON (system, power, temperature, comms)
 * - 1 línea por tipo cada 750ms
 * - Total: 3 segundos entre ciclos
 */

#include <Arduino.h>
#include <ESPCPUTemp.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Importar módulos de telemetría
#include "telemetry_storage.h"
#include "telemetry_generators.h"
#include "telemetry_transmission.h"
#include "telemetry_logger.h"
#include "telemetry_types.h"

/**
 * @brief Tarea que genera telemetría continuamente
 */
void vTelemetryGeneratorTask(void *pvParameters) {
  // Esperar a que setup() complete
  vTaskDelay(pdMS_TO_TICKS(100));
  
  Serial.println("\n[GENERATOR] Task started - generating telemetry every 750ms per type");
  
  for(;;) {
    // Generar System
    generate_system_telemetry();
    vTaskDelay(pdMS_TO_TICKS(750));
    
    // Generar Power
    generate_power_telemetry();
    vTaskDelay(pdMS_TO_TICKS(750));
    
    // Generar Temperature - ENVIADO DIRECTAMENTE POR SERIAL
    // (Evita problemas de serialización con storage)
    float obc_temp = 23.0f + (random(40) - 20) / 10.0f;
    float comms_temp = 24.0f + (random(40) - 20) / 10.0f;
    float payload_temp = 22.0f + (random(40) - 20) / 10.0f;
    float battery_temp = 25.0f + (random(40) - 20) / 10.0f;
    float external_temp = 20.0f + (random(40) - 20) / 10.0f;
    
    Serial.print("{\"type\":\"temperature\",\"obcTemp\":");
    Serial.print(obc_temp, 1);
    Serial.print(",\"commsTemp\":");
    Serial.print(comms_temp, 1);
    Serial.print(",\"payloadTemp\":");
    Serial.print(payload_temp, 1);
    Serial.print(",\"batteryTemp\":");
    Serial.print(battery_temp, 1);
    Serial.print(",\"externalTemp\":");
    Serial.print(external_temp, 1);
    Serial.println("}");
    
    vTaskDelay(pdMS_TO_TICKS(750));
    
    // Generar Subsystems (Comms)
    generate_subsystem_telemetry();
    vTaskDelay(pdMS_TO_TICKS(750));
    
    // Total: 3 segundos entre ciclos
  }
}

/**
 * @brief Tarea que transmite telemetría (envía por Serial en JSON)
 */
void vTelemetryTransmitterTask(void *pvParameters) {
  vTaskDelay(pdMS_TO_TICKS(200));
  
  Serial.println("[TRANSMITTER] Task started - sending JSON to Serial");
  
  for(;;) {
    // Procesar y enviar paquetes almacenados
    telemetry_transmission_cycle();
    
    // Pequeña pausa para evitar bloqueos
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n");
  Serial.println("============================================================");
  Serial.println("🛰️  TEIDESAT TELEMETRY SYSTEM - INTEGRATED MODE");
  Serial.println("============================================================");
  Serial.println("Using modular components (generators, storage, transmission)");
  Serial.println("Output format: JSON (system, power, temperature, comms)");
  Serial.println("Cycle time: 3 seconds");
  Serial.println("============================================================\n");
  
  // Inicializar módulos
  telemetry_storage_init();
  telemetry_logger_init();
  telemetry_transmission_init();
  
  Serial.println("[SETUP] Modules initialized");
  Serial.println("[SETUP] Starting telemetry tasks...\n");
  
  // Crear tareas (stack sizes en bytes)
  xTaskCreate(
    vTelemetryGeneratorTask,      // Función de la tarea
    "TelemetryGenerator",          // Nombre para debugging
    4096,                          // Stack size
    NULL,                          // Parámetros
    3,                             // Prioridad (3 = alta)
    NULL                           // Handle
  );
  
  xTaskCreate(
    vTelemetryTransmitterTask,
    "TelemetryTransmitter",
    4096,
    NULL,
    2,                             // Prioridad (2 = media)
    NULL
  );
  
  Serial.println("[SETUP] Scheduler starting...\n");
}

/**
 * @brief Loop de Arduino (no usado en FreeRTOS, pero requerido)
 */
void loop() {
  // FreeRTOS está en control del scheduler
  // Este loop no se ejecuta en modo FreeRTOS tradicional
  vTaskDelay(pdMS_TO_TICKS(10000));
}
