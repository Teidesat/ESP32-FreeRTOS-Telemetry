package com.teidesat.fomalhaut.repository;

import com.teidesat.fomalhaut.model.Telemetry;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.stereotype.Repository;

import java.util.List;

/**
 * TelemetryRepository
 * Data access layer for Telemetry entities
 */
@Repository
public interface TelemetryRepository extends JpaRepository<Telemetry, Long> {
    
    // Find logs by type
    List<Telemetry> findByTypeOrderByCreatedAtDesc(String type);
    
    // Find logs by type with pagination
    Page<Telemetry> findByTypeOrderByCreatedAtDesc(String type, Pageable pageable);
    
    // Find all logs ordered by creation date
    List<Telemetry> findAllByOrderByCreatedAtDesc();
    
    // Find latest N logs
    @Query(value = "SELECT * FROM telemetry_data ORDER BY created_at DESC LIMIT ?1", nativeQuery = true)
    List<Telemetry> findLatest(int count);
}
