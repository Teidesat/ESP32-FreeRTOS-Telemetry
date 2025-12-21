package com.teidesat.fomalhaut.controller;

import com.teidesat.fomalhaut.model.Telemetry;
import com.teidesat.fomalhaut.repository.TelemetryRepository;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * TelemetryController
 *
 * Controlador REST que expone los endpoints para recibir y consultar
 * telemetrías del sistema TeideSat. Los endpoints cuelgan de
 * `/api/telemetry` (ver `application.properties`).
 *
 * Flujo típico:
 * - El bridge Python envía `POST` con JSONs (por tipo o general).
 * - Se persiste la entidad `Telemetry` vía `TelemetryRepository`.
 * - Las consultas `GET` devuelven listas ordenadas por `createdAt`.
 */
@RestController
@RequestMapping("/telemetry")
@Slf4j
@CrossOrigin(origins = "*")
public class TelemetryController {
    
    @Autowired
    private TelemetryRepository telemetryRepository;
    
    /**
     * GET /api/telemetry
     * Devuelve todas las telemetrías (ordenadas por creación descendente).
     */
    @GetMapping
    public ResponseEntity<List<Telemetry>> getAllTelemetry() {
        log.debug("📡 GET /telemetry - Fetching all telemetry logs");
        List<Telemetry> logs = telemetryRepository.findAllByOrderByCreatedAtDesc();
        log.info("✅ Retrieved {} telemetry logs", logs.size());
        return ResponseEntity.ok(logs);
    }
    
    /**
     * GET /api/telemetry/system
     * Devuelve telemetrías de tipo `system`.
     */
    @GetMapping("/system")
    public ResponseEntity<List<Telemetry>> getSystemTelemetry() {
        log.debug("💻 GET /telemetry/system - Fetching system telemetry");
        List<Telemetry> logs = telemetryRepository.findByTypeOrderByCreatedAtDesc("system");
        log.info("✅ Retrieved {} system logs", logs.size());
        return ResponseEntity.ok(logs);
    }
    
    /**
     * GET /api/telemetry/power
     * Devuelve telemetrías de tipo `power`.
     */
    @GetMapping("/power")
    public ResponseEntity<List<Telemetry>> getPowerTelemetry() {
        log.debug("🔋 GET /telemetry/power - Fetching power telemetry");
        List<Telemetry> logs = telemetryRepository.findByTypeOrderByCreatedAtDesc("power");
        log.info("✅ Retrieved {} power logs", logs.size());
        return ResponseEntity.ok(logs);
    }
    
    /**
     * GET /api/telemetry/temperature
     * Devuelve telemetrías de tipo `temperature`.
     */
    @GetMapping("/temperature")
    public ResponseEntity<List<Telemetry>> getTemperatureTelemetry() {
        log.debug("🌡️  GET /telemetry/temperature - Fetching temperature telemetry");
        List<Telemetry> logs = telemetryRepository.findByTypeOrderByCreatedAtDesc("temperature");
        log.info("✅ Retrieved {} temperature logs", logs.size());
        return ResponseEntity.ok(logs);
    }
    
    /**
     * GET /api/telemetry/comms
     * Devuelve telemetrías de tipo `comms`.
     */
    @GetMapping("/comms")
    public ResponseEntity<List<Telemetry>> getCommsTelemetry() {
        log.debug("📡 GET /telemetry/comms - Fetching comms telemetry");
        List<Telemetry> logs = telemetryRepository.findByTypeOrderByCreatedAtDesc("comms");
        log.info("✅ Retrieved {} comms logs", logs.size());
        return ResponseEntity.ok(logs);
    }
    
    /**
     * GET /api/telemetry/latest/{count}
     * Devuelve las últimas `count` telemetrías.
     */
    @GetMapping("/latest/{count}")
    public ResponseEntity<List<Telemetry>> getLatestTelemetry(@PathVariable int count) {
        log.debug("📋 GET /telemetry/latest/{} - Fetching latest logs", count);
        List<Telemetry> logs = telemetryRepository.findLatest(count);
        log.info("✅ Retrieved {} latest logs", logs.size());
        return ResponseEntity.ok(logs);
    }
    
    /**
     * POST /api/telemetry/system
     * Inserta telemetría de tipo `system` enviada por el bridge.
     */
    @PostMapping("/system")
    public ResponseEntity<Map<String, Object>> receiveSystemTelemetry(@RequestBody Telemetry telemetry) {
        log.info("💻 Received system telemetry: CPU={}%, Memory={}B", 
                 telemetry.getCpuUsage(), telemetry.getMemoryFree());
        return saveTelemetry(telemetry);
    }
    
    /**
     * POST /api/telemetry/power
     * Inserta telemetría de tipo `power` enviada por el bridge.
     */
    @PostMapping("/power")
    public ResponseEntity<Map<String, Object>> receivePowerTelemetry(@RequestBody Telemetry telemetry) {
        log.info("🔋 Received power telemetry: V={}V, I={}A", 
                 telemetry.getVoltage(), telemetry.getCurrent());
        return saveTelemetry(telemetry);
    }
    
    /**
     * POST /api/telemetry/temperature
     * Inserta telemetría de tipo `temperature` enviada por el bridge.
     */
    @PostMapping("/temperature")
    public ResponseEntity<Map<String, Object>> receiveTemperatureTelemetry(@RequestBody Telemetry telemetry) {
        log.info("🌡️  Received temperature telemetry: OBC={}°C, Comms={}°C", 
                 telemetry.getObcTemp(), telemetry.getCommsTemp());
        return saveTelemetry(telemetry);
    }
    
    /**
     * POST /api/telemetry/comms
     * Inserta telemetría de tipo `comms` enviada por el bridge.
     */
    @PostMapping("/comms")
    public ResponseEntity<Map<String, Object>> receiveCommsTelemetry(@RequestBody Telemetry telemetry) {
        log.info("📡 Received comms telemetry: RSSI={}dBm, SNR={}dB", 
                 telemetry.getRssi(), telemetry.getSnr());
        return saveTelemetry(telemetry);
    }
    
    /**
     * POST /api/telemetry
     * Inserta telemetría general (sin tipo específico) enviada por el bridge.
     */
    @PostMapping
    public ResponseEntity<Map<String, Object>> receiveGeneralTelemetry(@RequestBody Telemetry telemetry) {
        log.info("📊 Received general telemetry: {}", telemetry.getRawLine());
        return saveTelemetry(telemetry);
    }
    
    /**
     * DELETE /api/telemetry/clear
     * Elimina todas las telemetrías.
     */
    @DeleteMapping("/clear")
    public ResponseEntity<Map<String, String>> clearAllTelemetry() {
        log.warn("🗑️  Clearing all telemetry logs");
        telemetryRepository.deleteAll();
        return ResponseEntity.ok(Map.of(
            "status", "success",
            "message", "All telemetry logs cleared"
        ));
    }
    
    /**
     * DELETE /api/telemetry/{id}
     * Elimina una telemetría por ID.
     */
    @DeleteMapping("/{id}")
    public ResponseEntity<Map<String, String>> deleteTelemetry(@PathVariable Long id) {
        telemetryRepository.deleteById(id);
        log.info("🗑️  Deleted telemetry log with ID: {}", id);
        return ResponseEntity.ok(Map.of(
            "status", "success",
            "message", "Telemetry log deleted"
        ));
    }
    
    /**
     * Guarda la entidad `Telemetry` y devuelve respuesta estándar.
     * Status: `201 Created` con `{status, message, id, type}`.
     */
    private ResponseEntity<Map<String, Object>> saveTelemetry(Telemetry telemetry) {
        Telemetry saved = telemetryRepository.save(telemetry);
        log.debug("✅ Telemetry saved with ID: {}", saved.getId());
        
        Map<String, Object> response = new HashMap<>();
        response.put("status", "received");
        response.put("message", "Telemetry data stored successfully");
        response.put("id", saved.getId());
        response.put("type", saved.getType());
        
        return ResponseEntity.status(HttpStatus.CREATED).body(response);
    }
}
