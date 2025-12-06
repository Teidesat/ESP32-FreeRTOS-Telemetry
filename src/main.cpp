/**
 * @file main.cpp
 * @brief Sistema de Telemetría Simplificado para Fomalhaut
 * 
 * Envía datos JSON directamente por Serial cada 3 segundos
 * Sin buffers, sin tareas complejas, sin ventanas de contacto.
 * Solo: generar → enviar por Serial
 */

#include <Arduino.h>
#include <ESPCPUTemp.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void send_system_data() {
  // VALORES REALES
  uint32_t free_heap = esp_get_free_heap_size();  // RAM libre real
  uint8_t task_count = uxTaskGetNumberOfTasks();   // Tareas reales
  float cpu_temp = temperatureRead();              // Temperatura CPU real
  
  // VALORES SIMULADOS
  uint8_t cpu_usage = 35 + random(20);  // 35-55%
  uint32_t uptime = millis() / 1000;
  
  Serial.print("{\"type\":\"system\",\"cpuUsage\":");
  Serial.print(cpu_usage);
  Serial.print(",\"memoryFree\":");
  Serial.print(free_heap);
  Serial.print(",\"uptime\":");
  Serial.print(uptime);
  Serial.print(",\"taskCount\":");
  Serial.print(task_count);
  Serial.print(",\"cpuTemp\":");
  Serial.print(cpu_temp, 1);
  Serial.println("}");
}

void send_power_data() {
  // Voltaje de batería: 3.3V ± 0.05V (variación realista)
  float voltage = 3.25 + (random(100) / 1000.0);  // 3.25-3.35V
  
  // Corriente: 0.45-0.55A
  float current = 0.45 + (random(100) / 1000.0);
  
  // Voltaje solar: 4.8-5.2V
  float solar_voltage = 4.8 + (random(40) / 100.0);
  
  // Corriente solar: 0.15-0.25A
  float solar_current = 0.15 + (random(100) / 1000.0);
  
  // Nivel de batería: 80-90%
  int battery_level = 80 + random(11);
  
  // Temperatura batería: 25°C ± 3°C
  int battery_temp = 22 + random(7);
  
  Serial.print("{\"type\":\"power\",\"voltage\":");
  Serial.print(voltage, 2);
  Serial.print(",\"current\":");
  Serial.print(current, 3);
  Serial.print(",\"solarVoltage\":");
  Serial.print(solar_voltage, 2);
  Serial.print(",\"solarCurrent\":");
  Serial.print(solar_current, 3);
  Serial.print(",\"batteryLevel\":");
  Serial.print(battery_level);
  Serial.print(",\"batteryTemp\":");
  Serial.print(battery_temp);
  Serial.println("}");
}

void send_temperature_data() {
  // Temperaturas simuladas con variación realista
  float obc_temp = 23.0 + (random(40) - 20) / 10.0;      // 21-25°C
  float comms_temp = 24.0 + (random(40) - 20) / 10.0;    // 22-26°C
  float payload_temp = 22.0 + (random(40) - 20) / 10.0;  // 20-24°C
  float battery_temp = 25.0 + (random(40) - 20) / 10.0;  // 23-27°C
  float external_temp = 20.0 + (random(40) - 20) / 10.0; // 18-22°C
  
  Serial.print("{\"type\":\"temperature\",\"obcTemp\":");
  Serial.print(obc_temp, 1);
  Serial.print(",\"commsTemp\":");
  Serial.print(comms_temp, 1);
  Serial.print(",\"payloadTemp\":");
  Serial.print(payload_temp, 1);
  Serial.print(",\"batteryTempFloat\":");
  Serial.print(battery_temp, 1);
  Serial.print(",\"externalTemp\":");
  Serial.print(external_temp, 1);
  Serial.println("}");
}

void send_comms_data() {
  // RSSI: -50 a -80 dBm (típico de enlaces satelitales)
  int rssi = -50 - random(31);
  
  // SNR: 8-18 dB (buena calidad)
  int snr = 8 + random(11);
  
  // Success rate: 92-98%
  int success_rate = 92 + random(7);
  
  Serial.print("{\"type\":\"comms\",\"rssi\":");
  Serial.print(rssi);
  Serial.print(",\"snr\":");
  Serial.print(snr);
  Serial.print(",\"commsUptime\":");
  Serial.print(millis() / 1000);
  Serial.print(",\"successRate\":");
  Serial.print(success_rate);
  Serial.println("}");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n");
  Serial.println("============================================================");
  Serial.println("🛰️  TEIDESAT TELEMETRY SYSTEM - SIMPLE MODE");
  Serial.println("============================================================");
  Serial.println("Enviando datos JSON cada 3 segundos...");
  Serial.println("============================================================\n");
}

void loop() {
  // Enviar 4 tipos de telemetría
  send_system_data();
  delay(750);
  
  send_power_data();
  delay(750);
  
  send_temperature_data();
  delay(750);
  
  send_comms_data();
  delay(750);
  
  // Total: 3 segundos entre ciclos
}
