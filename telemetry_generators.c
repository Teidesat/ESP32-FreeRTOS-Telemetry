/**
 * @file telemetry_generators.c
 * @brief Generadores de datos de telemetría
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 30-10-2025
 * 
 * @details
 * Este archivo contiene las funciones generadoras de datos de telemetría
 * para el sistema del satélite TeideSat. Las funciones simulan la lectura
 * de sensores y generan datos de telemetría estructurados que representan
 * el estado actual de los diferentes subsistemas del satélite.
 * 
 * El sistema genera cuatro tipos principales de telemetría:
 * - Estado del sistema (uptime, memoria, tareas)
 * - Datos de potencia (voltaje, corriente, nivel de batería)
 * - Temperaturas (OBC, comunicaciones, payload, batería, externa)
 * - Estado de subsistemas (comms, ADCS, payload, potencia)
 * 
 * @note En entorno WOKWI, se utilizan sensores virtuales que generan datos
 * aleatorios realistas. En hardware real, estas funciones se modificarían
 * para leer sensores físicos reales.
 */

#include "telemetry_storage.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_heap_caps.h"

static uint16_t sequence_number = 0; /**< Contador de secuencia para paquetes de telemetría */
static uint32_t system_uptime = 0; /**< Tiempo de actividad del sistema en segundos */

// Usar ADC del ESP32 para simular sensores (en WOKWI)
#ifdef WOKWI
/**
 * @brief Simula la lectura de voltaje de batería en entorno WOKWI
 * @return Voltaje simulado en el rango 3.300V - 3.499V
 * 
 * @details
 * Genera un voltaje de batería simulado con pequeñas fluctuaciones
 * aleatorias alrededor de 3.3V nominales, simulando el comportamiento
 * real de una batería de litio en condiciones normales de operación.
 * 
 * @code
 * // Ejemplo de uso:
 * float voltage = read_virtual_voltage();
 * // Resultado: 3.345V, 3.398V, 3.312V, etc.
 * @endcode
 */
float read_virtual_voltage() {
  return 3.3f + ((esp_random() % 200) / 1000.0f); // 3.3V ± 0.2V
  //        ↑              ↑           ↑
  //   Voltaje base    Variación     Escala
  //   nominal (3.3V)  (0-199)      (/1000 = 0.000-0.199V)
}

/**
 * @brief Simula la lectura de temperatura en entorno WOKWI
 * @return Temperatura simulada en el rango 25.0°C - 39.9°C
 * 
 * @details
 * Genera temperaturas simuladas con variaciones aleatorias alrededor
 * de 25°C, representando condiciones térmicas típicas en órbita LEO
 * (Low Earth Orbit) con fluctuaciones debido a la exposición solar.
 * 
 * @code
 * // Ejemplo de uso:
 * float temp = read_virtual_temperature();
 * // Resultado: 31.2°C, 28.7°C, 35.9°C, etc.
 * @endcode
 */
float read_virtual_temperature() {
  return 25.0f + ((esp_random() % 150) / 10.0f); // 25°C ± 15°C
  //        ↑              ↑           ↑
  //   Temp base      Variación      Escala
  //   (25°C)        (0-149)       (/10 = 0.0-14.9°C)
}
#endif

/**
 * @brief Genera datos de telemetría del estado del sistema
 * 
 * @details
 * Recopila y genera información sobre el estado general del sistema:
 * - Tiempo de actividad (uptime)
 * - Modo de operación del sistema
 * - Uso de memoria heap disponible
 * - Número de tareas activas en FreeRTOS
 * 
 * @note Los datos de uso de CPU no están disponibles en ESP32 FreeRTOS
 */

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

  // temperatura CPU ESP32
  system_telem.cpu_temperature = temperatureRead();
  
  telemetry_store_packet((telemetry_packet_t*)&system_telem);
}

/**
 * @brief Genera datos de telemetría del sistema de potencia
 * 
 * @details
 * Recopila información sobre el estado del sistema de potencia:
 * - Voltaje y corriente de batería
 * - Voltaje y corriente de paneles solares  
 * - Nivel de carga de la batería
 * - Temperatura de la batería
 * - Estado general del sistema de potencia
 * 
 * Esta telemetría tiene prioridad alta debido a su importancia crítica
 * En WOKWI se utilizan sensores virtuales, mientras que en hardware 
 * real se leerían ADCs y sensores I2C.
 * 
 * @note El nivel de batería disminuye gradualmente con el tiempo
 * para simular el consumo energético real.
 */
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

/**
 * @brief Genera datos de telemetría de temperaturas
 * 
 * @details
 * Recopila temperaturas de los diferentes componentes del satélite:
 * - OBC (On-Board Computer)
 * - Módulo de comunicaciones  
 * - Payload (carga útil)
 * - Batería
 * - Temperatura externa
 * 
 * Las temperaturas se generan para cada componente,
 * simulando los gradientes térmicos típicos en un satélite.
 * 
 * @note En un sistema real, estos datos vendrían de sensores de temperatura
 * como termistores o sensores I2C (DS18B20, TMP36, etc.).
 */
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

/**
 * @brief Genera datos de telemetría del estado de subsistemas
 * 
 * @details
 * Proporciona información sobre el estado operativo de los principales
 * subsistemas del satélite:
 * - Estado de comunicaciones (operacional/fallo)
 * - Estado del ADCS 
 * - Estado del payload 
 * - Estado del sistema de potencia
 * - Tiempos de actividad de subsistemas
 * - Estadísticas de ejecución de comandoss
 */
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