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
 *
 * Capa de acceso a datos para la entidad `Telemetry`.
 * Incluye métodos derivados de nombre (Spring Data) y una consulta
 * nativa para obtener las últimas N entradas.
 */
@Repository
public interface TelemetryRepository extends JpaRepository<Telemetry, Long> {
    
    /** Busca por tipo y ordena por creación descendente */
    List<Telemetry> findByTypeOrderByCreatedAtDesc(String type);
    
    /** Busca por tipo con paginación y orden descendente */
    Page<Telemetry> findByTypeOrderByCreatedAtDesc(String type, Pageable pageable);
    
    /** Devuelve todas las entradas ordenadas por creación descendente */
    List<Telemetry> findAllByOrderByCreatedAtDesc();
    
    /** Consulta nativa: últimas `count` entradas */
    @Query(value = "SELECT * FROM telemetry_data ORDER BY created_at DESC LIMIT ?1", nativeQuery = true)
    List<Telemetry> findLatest(int count);
}
