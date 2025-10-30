#include "telemetry_storage.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

// Tarea: Recolector de telemetría
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

// Tarea: Procesador de telemetría 
void vTelemetryProcessorTask(void *pvParameters) {
    telemetry_packet_t packet;
    uint32_t processed_count = 0;
    
    printf("🔧 Telemetry Processor Task Started\n");
    
    for(;;) {
        if(telemetry_retrieve_packet(&packet)) {
            processed_count++;
            
            // Visualización mejorada para WOKWI
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

// Tarea: Simulador de transmisión (para WOKWI)
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