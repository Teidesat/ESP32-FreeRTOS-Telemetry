package com.teidesat.fomalhaut;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;
import org.springframework.web.servlet.config.annotation.CorsRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;

/**
 * Fomalhaut Backend Application
 *
 * Punto de entrada principal del backend de Fomalhaut.
 * Configura CORS de forma abierta para facilitar el desarrollo
 * (orígenes `*`, métodos básicos), y arranca la aplicación.
 *
 * Nota: para producción, se recomienda restringir `allowedOrigins`
 * a los dominios necesarios.
 */
@SpringBootApplication
public class FomalhautBackendApplication {

    public static void main(String[] args) {
        SpringApplication.run(FomalhautBackendApplication.class, args);
    }

    /**
     * Configuración CORS global.
     *
     * Permite peticiones desde cualquier origen y métodos estándar.
     * Facilita que el bridge (Python) y UIs web accedan a los endpoints.
     */
    @Bean
    public WebMvcConfigurer corsConfigurer() {
        return new WebMvcConfigurer() {
            @Override
            public void addCorsMappings(CorsRegistry registry) {
                registry.addMapping("/**")
                        .allowedOrigins("*")
                        .allowedMethods("GET", "POST", "PUT", "DELETE", "OPTIONS")
                        .allowedHeaders("*")
                        .allowCredentials(false)
                        .maxAge(3600);
            }
        };
    }
}
