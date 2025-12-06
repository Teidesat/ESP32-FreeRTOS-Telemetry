#!/bin/bash
# Script para iniciar el sistema completo: Backend + Frontend + Bridge

set -e

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Funciones de utilidad
print_header() {
    echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║${NC} $1"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"
}

print_ok() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_warn() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

# Verificar requisitos
check_requirements() {
    print_header "Verificando Requisitos"
    
    # Java
    if command -v java &> /dev/null; then
        JAVA_VERSION=$(java -version 2>&1 | grep -oP 'version "\K[^"]*')
        print_ok "Java instalado: $JAVA_VERSION"
    else
        print_error "Java no encontrado. Instala OpenJDK 17+"
        exit 1
    fi
    
    # Maven
    if command -v mvn &> /dev/null; then
        MVN_VERSION=$(mvn -v 2>/dev/null | head -n 1)
        print_ok "Maven instalado: $MVN_VERSION"
    else
        print_warn "Maven no encontrado. Algunos scripts pueden fallar."
    fi
    
    # Python3
    if command -v python3 &> /dev/null; then
        PYTHON_VERSION=$(python3 --version 2>&1)
        print_ok "Python3 instalado: $PYTHON_VERSION"
    else
        print_error "Python3 no encontrado"
        exit 1
    fi
    
    # Docker (opcional)
    if command -v docker &> /dev/null; then
        DOCKER_VERSION=$(docker --version 2>&1)
        print_ok "Docker instalado: $DOCKER_VERSION"
        USE_DOCKER=true
    else
        print_warn "Docker no encontrado. Usaremos Maven local."
        USE_DOCKER=false
    fi
    
    echo ""
}

# Compilar backend
compile_backend() {
    print_header "Compilando Backend Java-Spring"
    
    if [ -z "$USE_DOCKER" ] || [ "$USE_DOCKER" = "false" ]; then
        cd backend
        print_info "Compilando con Maven..."
        mvn clean package -q
        print_ok "Backend compilado exitosamente"
        cd ..
    else
        print_info "Backend será compilado en Docker"
    fi
    echo ""
}

# Iniciar backend
start_backend() {
    print_header "Iniciando Backend (Puerto 20001)"
    
    if [ "$USE_DOCKER" = "true" ]; then
        print_info "Iniciando con Docker Compose..."
        docker-compose up -d backend
        sleep 5
        
        # Verificar que está corriendo
        if docker ps | grep -q fomalhaut-backend; then
            print_ok "Backend corriendo en Docker"
        else
            print_error "Error al iniciar backend"
            docker-compose logs backend
            exit 1
        fi
    else
        print_info "Iniciando localmente con Maven..."
        cd backend
        java -jar target/fomalhaut-backend-1.0.0.jar > /tmp/backend.log 2>&1 &
        BACKEND_PID=$!
        cd ..
        echo $BACKEND_PID > /tmp/backend.pid
        
        # Esperar a que esté listo
        print_info "Esperando a que backend esté listo..."
        for i in {1..30}; do
            if curl -s http://localhost:20001/api/telemetry > /dev/null 2>&1; then
                print_ok "Backend corriendo (PID: $BACKEND_PID)"
                break
            fi
            if [ $i -eq 30 ]; then
                print_error "Backend no respondió después de 30 segundos"
                tail -20 /tmp/backend.log
                exit 1
            fi
            sleep 1
        done
    fi
    echo ""
}

# Instalar dependencias Python
install_bridge_deps() {
    print_header "Preparando Bridge Python"
    
    cd bridge
    
    if ! python3 -c "import serial, requests" 2>/dev/null; then
        print_info "Instalando dependencias Python..."
        pip install -r requirements.txt -q
        print_ok "Dependencias instaladas"
    else
        print_ok "Dependencias Python ya instaladas"
    fi
    
    cd ..
    echo ""
}

# Iniciar frontend
start_frontend() {
    print_header "Iniciando Frontend Fomalhaut (Puerto 20002)"
    
    if [ "$USE_DOCKER" = "true" ]; then
        print_info "Iniciando con Docker Compose..."
        docker-compose up -d frontend
        sleep 3
        
        if docker ps | grep -q fomalhaut-frontend; then
            print_ok "Frontend corriendo en Docker"
        fi
    else
        print_warn "Docker no disponible para frontend"
        print_info "Inicia manualmente:"
        print_info "  cd Fomalhaut && npm run dev"
    fi
    echo ""
}

# Mostrar opciones
show_options() {
    print_header "Sistema Fomalhaut - Opciones"
    
    echo ""
    echo "🚀 El sistema está en marcha:"
    echo ""
    echo "📊 Backend:"
    echo "   URL: http://localhost:20001/api"
    echo "   Estado: $(curl -s http://localhost:20001/api/telemetry > /dev/null && echo '✅ Corriendo' || echo '❌ Error')"
    echo ""
    echo "🖥️  Frontend:"
    if [ "$USE_DOCKER" = "true" ]; then
        echo "   URL: http://localhost:20002"
        echo "   Estado: $(docker ps | grep -q fomalhaut-frontend && echo '✅ Corriendo' || echo '⏸️  No iniciado')"
    else
        echo "   Inicia manualmente: cd Fomalhaut && npm run dev"
    fi
    echo ""
    echo "📡 Bridge Python:"
    echo "   Para conectar ESP32: cd bridge && python3 esp32_to_fomalhaut_bridge.py"
    echo "   O para simular: cd bridge && python3 simulate_esp32.py"
    echo ""
    echo "🧪 Testing:"
    echo "   Simular ESP32: cd bridge && python3 simulate_esp32.py"
    echo "   Ver logs backend: docker logs -f fomalhaut-backend (con Docker)"
    echo ""
    echo "📝 Consulta los logs en Frontend → Logs Tab"
    echo ""
    echo "Presiona Ctrl+C para detener"
    echo ""
}

# Monitorear
monitor_system() {
    echo ""
    
    # Verificar backend cada 5 segundos
    while true; do
        if ! curl -s http://localhost:20001/api/telemetry > /dev/null 2>&1; then
            print_error "Backend no responde. Verifica los logs."
            break
        fi
        sleep 5
    done
}

# Limpieza
cleanup() {
    print_header "Deteniendo Sistema"
    
    if [ "$USE_DOCKER" = "true" ]; then
        print_info "Deteniendo contenedores Docker..."
        docker-compose down
    else
        print_info "Deteniendo procesos locales..."
        if [ -f /tmp/backend.pid ]; then
            kill $(cat /tmp/backend.pid) 2>/dev/null || true
            rm /tmp/backend.pid
        fi
    fi
    
    print_ok "Sistema detenido"
}

# Main
main() {
    print_header "FOMALHAUT - SISTEMA COMPLETO"
    echo ""
    
    # Trap para limpiar al salir
    trap cleanup EXIT INT TERM
    
    # Ejecutar pasos
    check_requirements
    
    # Preguntar por modo
    echo "¿Cómo deseas ejecutar el sistema?"
    echo "1) Docker Compose (recomendado)"
    echo "2) Local con Maven"
    echo ""
    read -p "Selecciona (1 o 2): " mode
    
    if [ "$mode" = "1" ] && [ "$USE_DOCKER" = "true" ]; then
        print_info "Usando Docker Compose..."
        docker-compose up -d
        sleep 5
    else
        compile_backend
        start_backend
    fi
    
    install_bridge_deps
    
    if [ "$USE_DOCKER" = "true" ]; then
        docker-compose up -d frontend 2>/dev/null || true
    fi
    
    show_options
    monitor_system
}

# Ejecutar
main "$@"
