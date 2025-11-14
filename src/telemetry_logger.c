/*
#include "../include/telemetry_logger.h"

bool telemetry_logger_init(void) {
  if(!LittleFS.begin(true)) {
    printf("❌ LittleFS mount failed\n");
    return false;
  }
  printf("📂 LittleFS mounted successfully\n");
  return true;
}
void telemetry_log_to_file(const char *entry) {
  File file = LittleFS.open("/telemetry_log.txt", FILE_APPEND);
  if(!file) {
    printf("❌ Failed to open telemetry log file for appending\n");
    return;
  }
  file.println(entry);
  file.close();
  printf("✅ Logged telemetry entry to file\n");
}
*/