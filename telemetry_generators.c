#include "telemetry_storage.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_heap_caps.h"

static uint16_t sequence_number = 0;
static uint32_t system_uptime = 0;

// Usar ADC del ESP32 para simular sensores (en WOKWI)
#ifdef WOKWI
// En WOKWI podemos usar "sensores virtuales"
float read_virtual_voltage() {
    return 3.3f + ((esp_random() % 200) / 1000.0f); // 3.3V ± 0.2V
}

float read_virtual_temperature() {
    return 25.0f + ((esp_random() % 150) / 10.0f); // 25°C ± 15°C
}
#endif

void generate_system_telemetry(void) {
    system_status_telem_t system_telem;
    
    system_telem.header.type = TELEM_SYSTEM_STATUS;
    system_telem.header.timestamp = xTaskGetTickCount();
    system_telem.header.sequence = sequence_number++;
    system_telem.header.priority = 1;
    
    system_telem.uptime_seconds = system_uptime++;
    
    // Estados específicos del ESP32
    system_telem.system_mode = 1; // nominal
    system_telem.cpu_usage = 0;   // En ESP32 no tenemos esta métrica fácil
    system_telem.stack_high_water = uxTaskGetStackHighWaterMark(NULL);
    
    // Memoria ESP32
    system_telem.heap_free = esp_get_free_heap_size();
    system_telem.task_count = uxTaskGetNumberOfTasks();
    system_telem.error_count = 0;
    
    telemetry_store_packet((telemetry_packet_t*)&system_telem);
}

void generate_power_telemetry(void) {
    power_telem_t power_telem;
    
    power_telem.header.type = TELEM_POWER_DATA;
    power_telem.header.timestamp = xTaskGetTickCount();
    power_telem.header.sequence = sequence_number++;
    power_telem.header.priority = 2;
    
#ifdef WOKWI
    power_telem.battery_voltage = read_virtual_voltage();
    power_telem.battery_temperature = read_virtual_temperature();
#else
    power_telem.battery_voltage = 3.3f;
    power_telem.battery_temperature = 25;
#endif
    
    power_telem.battery_current = 0.1f;
    power_telem.solar_panel_voltage = 5.0f;
    power_telem.solar_panel_current = 0.5f;
    power_telem.battery_level = 85 - (system_uptime / 3600);
    power_telem.power_state = 0;
    
    telemetry_store_packet((telemetry_packet_t*)&power_telem);
}

void generate_temperature_telemetry(void) {
    temperature_telem_t temp_telem;
    
    temp_telem.header.type = TELEM_TEMPERATURE_DATA;
    temp_telem.header.timestamp = xTaskGetTickCount();
    temp_telem.header.sequence = sequence_number++;
    temp_telem.header.priority = 1;
    
#ifdef WOKWI
    temp_telem.obc_temperature = read_virtual_temperature();
    temp_telem.comms_temperature = read_virtual_temperature() - 5;
    temp_telem.payload_temperature = read_virtual_temperature() + 3;
    temp_telem.battery_temperature = read_virtual_temperature();
    temp_telem.external_temperature = read_virtual_temperature() - 10;
#else
    temp_telem.obc_temperature = 35;
    temp_telem.comms_temperature = 28;
    temp_telem.payload_temperature = 25;
    temp_telem.battery_temperature = 22;
    temp_telem.external_temperature = -15;
#endif
    
    telemetry_store_packet((telemetry_packet_t*)&temp_telem);
}

void generate_subsystem_telemetry(void) {
    subsystem_status_telem_t subsys_telem;
    
    subsys_telem.header.type = TELEM_COMMUNICATION_STATUS;
    subsys_telem.header.timestamp = xTaskGetTickCount();
    subsys_telem.header.sequence = sequence_number++;
    subsys_telem.header.priority = 1;
    
    subsys_telem.comms_status = 1;
    subsys_telem.adcs_status = 1;  
    subsys_telem.payload_status = 1;
    subsys_telem.power_status = 1;
    subsys_telem.comms_uptime = system_uptime;
    subsys_telem.payload_uptime = system_uptime - 100;
    subsys_telem.last_command_id = 0x25;
    subsys_telem.command_success_rate = 98;
    
    telemetry_store_packet((telemetry_packet_t*)&subsys_telem);
}