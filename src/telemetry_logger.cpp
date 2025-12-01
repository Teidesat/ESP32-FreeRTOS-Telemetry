/**
 * @file telemetry_logger.cpp
 * @brief Implementación del logger de telemetría en LittleFS
 * @author Aarón Ramírez Valencia - TeideSat
 * @date 10-11-2025
 * 
 * @details  Este módulo implementa un logger simple de telemetría
 * que escribe mensajes formateados en un archivo de LittleFS.
 * El logger también imprime los mensajes en el puerto serie. 
 */

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <stdarg.h>
#include "../include/telemetry_logger.h"

// Implementación mínima sin mutex (solo usada desde loop/setup)
static bool s_logger_ready = false;

bool telemetry_logger_init(void) {
  if (!LittleFS.begin(true)) {
    Serial.println("[Logger] ERROR montando LittleFS");
    return false;
  }
  s_logger_ready = true;
  Serial.printf("[Logger] OK. Archivo: %s\n", TELEMETRY_LOG_FILE);
  return true;
}

void telemetry_logf(const char *fmt, ...) {
  if (!s_logger_ready) return;
  char buffer[160];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  // Serial
  Serial.println(buffer);

  // Archivo
  File f = LittleFS.open(TELEMETRY_LOG_FILE, FILE_APPEND);
  if (f) {
    f.println(buffer);
    f.close();
  }
}

void telemetry_dump_log(void) {
  if (!s_logger_ready) {
    Serial.println("[Logger] No listo para dump");
    return;
  }
  File f = LittleFS.open(TELEMETRY_LOG_FILE, FILE_READ);
  if (!f) {
    Serial.println("[Logger] No se pudo abrir el log para lectura");
    return;
  }
  size_t sz = f.size();
  Serial.printf("\n[Logger] >>> BEGIN FILE DUMP: %s (size: %u bytes)\n", TELEMETRY_LOG_FILE, (unsigned)sz);
  Serial.println("[Logger] --- START ---");
  while (f.available()) {
    Serial.write(f.read());
  }
  f.close();
  Serial.println("[Logger] --- END ---");
  Serial.println("[Logger] <<< END FILE DUMP\n");
}

void telemetry_log_clear(void) {
  if (!s_logger_ready) return;
  // Truncar el archivo: abrir en FILE_WRITE y cerrar sin escribir
  File f = LittleFS.open(TELEMETRY_LOG_FILE, FILE_WRITE);
  if (f) {
    f.close();
    Serial.println("[Logger] Log truncado (archivo limpio)");
  } else {
    // Si no existe aún, no pasa nada
    Serial.println("[Logger] Log aún no existe; nada que truncar");
  }
}

// ============================================================================
// Funciones de logging específicas por tipo de telemetría
// ============================================================================

static void write_to_file(const char *filename, const char *buffer) {
  if (!s_logger_ready) return;
  File f = LittleFS.open(filename, FILE_APPEND);
  if (f) {
    f.println(buffer);
    f.close();
  }
}

void telemetry_log_system(const char *fmt, ...) {
  if (!s_logger_ready) return;
  char buffer[200];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  Serial.println(buffer);
  write_to_file(TELEMETRY_SYSTEM_LOG, buffer);
}

void telemetry_log_power(const char *fmt, ...) {
  if (!s_logger_ready) return;
  char buffer[200];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  Serial.println(buffer);
  write_to_file(TELEMETRY_POWER_LOG, buffer);
}

void telemetry_log_temperature(const char *fmt, ...) {
  if (!s_logger_ready) return;
  char buffer[200];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  Serial.println(buffer);
  write_to_file(TELEMETRY_TEMP_LOG, buffer);
}

void telemetry_log_comms(const char *fmt, ...) {
  if (!s_logger_ready) return;
  char buffer[200];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  Serial.println(buffer);
  write_to_file(TELEMETRY_COMMS_LOG, buffer);
}

// ============================================================================
// Funciones de volcado por tipo
// ============================================================================

static void dump_file(const char *filename, const char *label) {
  if (!s_logger_ready) {
    Serial.printf("[Logger] No listo para dump de %s\n", label);
    return;
  }
  File f = LittleFS.open(filename, FILE_READ);
  if (!f) {
    Serial.printf("[Logger] No se pudo abrir %s para lectura\n", filename);
    return;
  }
  size_t sz = f.size();
  Serial.printf("\n[Logger] >>> BEGIN %s DUMP: %s (%u bytes)\n", label, filename, (unsigned)sz);
  Serial.println("[Logger] --- START ---");
  while (f.available()) {
    Serial.write(f.read());
  }
  f.close();
  Serial.println("\n[Logger] --- END ---");
  Serial.printf("[Logger] <<< END %s DUMP\n\n", label);
}

void telemetry_dump_system_log(void) {
  dump_file(TELEMETRY_SYSTEM_LOG, "SYSTEM");
}

void telemetry_dump_power_log(void) {
  dump_file(TELEMETRY_POWER_LOG, "POWER");
}

void telemetry_dump_temperature_log(void) {
  dump_file(TELEMETRY_TEMP_LOG, "TEMPERATURE");
}

void telemetry_dump_comms_log(void) {
  dump_file(TELEMETRY_COMMS_LOG, "COMMS");
}

void telemetry_dump_all_logs(void) {
  Serial.println("\n╔═══════════════════════════════════════════════════════════╗");
  Serial.println("║         VOLCADO COMPLETO DE LOGS DE TELEMETRÍA            ║");
  Serial.println("╚═══════════════════════════════════════════════════════════╝\n");
  
  telemetry_dump_system_log();
  telemetry_dump_power_log();
  telemetry_dump_temperature_log();
  telemetry_dump_comms_log();
  
  Serial.println("╔═══════════════════════════════════════════════════════════╗");
  Serial.println("║              FIN DEL VOLCADO COMPLETO                     ║");
  Serial.println("╚═══════════════════════════════════════════════════════════╝\n");
}
