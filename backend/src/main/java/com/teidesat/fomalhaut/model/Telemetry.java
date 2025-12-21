package com.teidesat.fomalhaut.model;

import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;
import java.time.LocalDateTime;

/**
 * Entidad de datos de Telemetría.
 *
 * Representa una entrada de telemetría proveniente del ESP32/bridge.
 * Admite campos opcionales por tipo; no es necesario poblar todos
 * los atributos para persistir.
 *
 * Tabla: `telemetry_data`
 */
@Entity
@Table(name = "telemetry_data")
@Data
@NoArgsConstructor
@AllArgsConstructor
public class Telemetry {
    
    /** Identificador autogenerado */
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    
    /** Timestamp original del mensaje (puede venir del ESP32 o del bridge) */
    @Column(nullable = false)
    private String timestamp;
    
    /** Tipo de telemetría: system, power, temperature, comms, general */
    @Column(nullable = false)
    private String type;
    
    /** Línea cruda opcional, útil para auditoría/debug */
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
    
    /** Fecha de creación establecida automáticamente al persistir */
    @Column(name = "created_at", nullable = false, updatable = false)
    private LocalDateTime createdAt;
    
    @PrePersist
    protected void onCreate() {
        createdAt = LocalDateTime.now();
    }
}
