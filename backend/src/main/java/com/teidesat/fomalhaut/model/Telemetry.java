package com.teidesat.fomalhaut.model;

import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;
import java.time.LocalDateTime;

/**
 * Telemetry Data Entity
 * Represents a single telemetry log entry from the ESP32
 */
@Entity
@Table(name = "telemetry_data")
@Data
@NoArgsConstructor
@AllArgsConstructor
public class Telemetry {
    
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    
    @Column(nullable = false)
    private String timestamp;
    
    @Column(nullable = false)
    private String type; // system, power, temperature, comms, general
    
    @Column(columnDefinition = "TEXT")
    private String rawLine;
    
    // Campos específicos para cada tipo de telemetría
    
    // System telemetry
    private Integer cpuUsage;
    private Long memoryFree;
    private Long uptime;
    private Integer taskCount;
    private Float cpuTemp;
    
    // Power telemetry
    private Float voltage;
    private Float current;
    private Float solarVoltage;
    private Float solarCurrent;
    private Integer batteryLevel;
    private Integer batteryTemp;
    
    // Temperature telemetry
    private Float obcTemp;
    private Float commsTemp;
    private Float payloadTemp;
    private Float batteryTempFloat;  // Float version for temperature data
    private Float externalTemp;
    
    // Comms telemetry
    private Integer rssi;
    private Integer snr;
    private Long commsUptime;
    private Integer successRate;
    
    @Column(name = "created_at", nullable = false, updatable = false)
    private LocalDateTime createdAt;
    
    @PrePersist
    protected void onCreate() {
        createdAt = LocalDateTime.now();
    }
}
