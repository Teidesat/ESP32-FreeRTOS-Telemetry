/**
 * @file telemetry_tasks.c
 * @brief Tareas FreeRTOS del sistema de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 20-10-2025
 * 
 * @details
 * Este archivo contiene la implementación de las tareas FreeRTOS que componen
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

#include "telemetry_storage.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

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
void vTelemetryCollectorTask(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();

  telemetry_storage_init();
  printf("🚀 Telemetry Collector Task Started\n");

  for(;;) {
    generate_system_telemetry();
    generate_power_telemetry();
    generate_temperature_telemetry(); 
    generate_subsystem_telemetry();
    
    // En WOKWI podemos usar intervalos más cortos para ver datos rápido
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000)); // 5 segundos
  }
}

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
void vTelemetryProcessorTask(void *pvParameters) {
  telemetry_packet_t packet;
  uint32_t processed_count = 0;

  printf("🔧 Telemetry Processor Task Started\n");

  for(;;) {
    if(telemetry_retrieve_packet(&packet)) {
      processed_count++;

      // Visualización para WOKWI
      switch(packet.header.type) {
        case TELEM_SYSTEM_STATUS:
          printf("📊 SYSTEM: Uptime=%lus | Heap=%lu | Tasks=%d | Seq=%d\n",
                 packet.system.uptime_seconds,
                 packet.system.heap_free,
                 packet.system.task_count,
                 packet.header.sequence);
          break;

        case TELEM_POWER_DATA:
          printf("🔋 POWER: Bat=%.2fV | Level=%d%% | Temp=%dC | Seq=%d\n", 
                 packet.power.battery_voltage,
                 packet.power.battery_level,
                 packet.power.battery_temperature,
                 packet.header.sequence);
          break;

        case TELEM_TEMPERATURE_DATA:
          printf("🌡️ TEMP: OBC=%dC | COMMS=%dC | PAYLOAD=%dC | Seq=%d\n",
                 packet.temperature.obc_temperature,
                 packet.temperature.comms_temperature,
                 packet.temperature.payload_temperature, 
                 packet.header.sequence);
          break;

        case TELEM_COMMUNICATION_STATUS:
          printf("📡 COMMS: Status=%d | Uptime=%lu | Success=%d%% | Seq=%d\n",
            		 packet.subsystems.comms_status,
                 packet.subsystems.comms_uptime,
                 packet.subsystems.command_success_rate,
                 packet.header.sequence);
          break;
      }

      printf("   Available packets: %lu\n", telemetry_available_packets());

		} else {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}


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
void vTelemetryTransmitterTask(void *pvParameters) {
  telemetry_packet_t packet;
  bool ground_station_available = false;
  uint32_t transmission_count = 0;

	printf("📡 Telemetry Transmitter Task Started\n");

  for(;;) {
    // En WOKWI, simular disponibilidad aleatoria de estación terrestre
    if((xTaskGetTickCount() / 1000) % 30 == 0) { // Cada ~30 segundos
      ground_station_available = true;
      printf("\n🎯 GROUND STATION CONTACT WINDOW OPEN!\n");
    }

    if(ground_station_available) {
      uint32_t available = telemetry_available_packets();

      if(available > 0) {
        printf("📤 TRANSMITTING %lu packets to ground...\n", available);

        while(telemetry_retrieve_packet(&packet)) {
          transmission_count++;
          printf("   📦 [%lu] Type=%d, Seq=%d, Time=%lu\n",
                 transmission_count, packet.header.type, 
                 packet.header.sequence, packet.header.timestamp);

          // Pequeña pausa para simular transmisión
          vTaskDelay(pdMS_TO_TICKS(50));
        }

        printf("✅ Transmission complete. Total sent: %lu packets\n\n", transmission_count);
      }

      ground_station_available = false;
    }

    vTaskDelay(pdMS_TO_TICKS(2000)); // Revisar cada 2 segundos
  }
}