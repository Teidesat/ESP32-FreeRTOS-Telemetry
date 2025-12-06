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
 * REST API endpoints for telemetry data
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
     * Get all telemetry logs
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
     * Get all system telemetry logs
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
     * Get all power telemetry logs
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
     * Get all temperature telemetry logs
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
     * Get all communications telemetry logs
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
     * Get latest N telemetry logs
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
     * Receive system telemetry from bridge
     */
    @PostMapping("/system")
    public ResponseEntity<Map<String, Object>> receiveSystemTelemetry(@RequestBody Telemetry telemetry) {
        log.info("💻 Received system telemetry: CPU={}%, Memory={}B", 
                 telemetry.getCpuUsage(), telemetry.getMemoryFree());
        return saveTelemetry(telemetry);
    }
    
    /**
     * POST /api/telemetry/power
     * Receive power telemetry from bridge
     */
    @PostMapping("/power")
    public ResponseEntity<Map<String, Object>> receivePowerTelemetry(@RequestBody Telemetry telemetry) {
        log.info("🔋 Received power telemetry: V={}V, I={}A", 
                 telemetry.getVoltage(), telemetry.getCurrent());
        return saveTelemetry(telemetry);
    }
    
    /**
     * POST /api/telemetry/temperature
     * Receive temperature telemetry from bridge
     */
    @PostMapping("/temperature")
    public ResponseEntity<Map<String, Object>> receiveTemperatureTelemetry(@RequestBody Telemetry telemetry) {
        log.info("🌡️  Received temperature telemetry: OBC={}°C, Comms={}°C", 
                 telemetry.getObcTemp(), telemetry.getCommsTemp());
        return saveTelemetry(telemetry);
    }
    
    /**
     * POST /api/telemetry/comms
     * Receive communications telemetry from bridge
     */
    @PostMapping("/comms")
    public ResponseEntity<Map<String, Object>> receiveCommsTelemetry(@RequestBody Telemetry telemetry) {
        log.info("📡 Received comms telemetry: RSSI={}dBm, SNR={}dB", 
                 telemetry.getRssi(), telemetry.getSnr());
        return saveTelemetry(telemetry);
    }
    
    /**
     * POST /api/telemetry
     * Receive general telemetry from bridge
     */
    @PostMapping
    public ResponseEntity<Map<String, Object>> receiveGeneralTelemetry(@RequestBody Telemetry telemetry) {
        log.info("📊 Received general telemetry: {}", telemetry.getRawLine());
        return saveTelemetry(telemetry);
    }
    
    /**
     * DELETE /api/telemetry/clear
     * Clear all telemetry logs
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
     * Delete a specific telemetry log
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
     * Utility method to save telemetry data
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
