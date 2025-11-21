#include "../include/telemetry_logger.h"

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <stdarg.h>

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
