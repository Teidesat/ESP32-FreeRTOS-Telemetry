#!/bin/bash
# Script de testing completo del sistema ESP32 → Fomalhaut
# Ejecuta tests y proporciona información útil

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║     ESP32 → FOMALHAUT BRIDGE - TESTING & VALIDATION      ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Función para imprimir con color
print_status() {
    if [ "$1" == "OK" ]; then
        echo -e "${GREEN}✅ $2${NC}"
    elif [ "$1" == "ERROR" ]; then
        echo -e "${RED}❌ $2${NC}"
    elif [ "$1" == "INFO" ]; then
        echo -e "${BLUE}ℹ️  $2${NC}"
    elif [ "$1" == "WARN" ]; then
        echo -e "${YELLOW}⚠️  $2${NC}"
    fi
}

# 1. Verificar Python
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "1️⃣  Verificando Python"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if command -v python3 &> /dev/null; then
    PYTHON_VERSION=$(python3 --version 2>&1)
    print_status "OK" "Python instalado: $PYTHON_VERSION"
else
    print_status "ERROR" "Python3 no encontrado"
    echo "   Instala Python3: sudo apt install python3 python3-pip"
    exit 1
fi
echo ""

# 2. Verificar dependencias
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "2️⃣  Verificando dependencias Python"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

check_package() {
    if python3 -c "import $1" 2>/dev/null; then
        print_status "OK" "$1 instalado"
        return 0
    else
        print_status "ERROR" "$1 NO instalado"
        return 1
    fi
}

MISSING_DEPS=0
check_package "serial" || MISSING_DEPS=1
check_package "requests" || MISSING_DEPS=1

if [ $MISSING_DEPS -eq 1 ]; then
    echo ""
    print_status "WARN" "Instalando dependencias faltantes..."
    pip install -r requirements.txt
    echo ""
fi
echo ""

# 3. Ejecutar tests de parsing
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "3️⃣  Ejecutando tests de parsing"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
python3 test_bridge.py
if [ $? -eq 0 ]; then
    print_status "OK" "Tests de parsing pasados correctamente"
else
    print_status "ERROR" "Algunos tests fallaron"
fi
echo ""

# 4. Verificar puerto serial (si está conectado)
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "4️⃣  Verificando puertos seriales"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ "$(uname)" == "Linux" ]; then
    echo "Puertos disponibles:"
    if ls /dev/ttyUSB* 2>/dev/null || ls /dev/ttyACM* 2>/dev/null; then
        for port in /dev/ttyUSB* /dev/ttyACM*; do
            if [ -e "$port" ]; then
                print_status "INFO" "Encontrado: $port"
            fi
        done
    else
        print_status "WARN" "No se encontraron puertos USB/ACM"
        echo "   ¿Está conectado el ESP32?"
    fi
elif [ "$(uname)" == "Darwin" ]; then
    echo "Puertos disponibles:"
    ls /dev/cu.* 2>/dev/null || print_status "WARN" "No se encontraron puertos"
else
    print_status "INFO" "Sistema Windows detectado - verificar manualmente en Device Manager"
fi
echo ""

# 5. Verificar conectividad al servidor backend
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "5️⃣  Verificando servidor backend"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Leer URL del config.json
if [ -f "config.json" ]; then
    SERVER_URL=$(python3 -c "import json; print(json.load(open('config.json'))['server']['base_url'])")
    echo "URL del servidor: $SERVER_URL"
    
    # Intentar conectar
    if command -v curl &> /dev/null; then
        if curl -s --connect-timeout 3 "$SERVER_URL" > /dev/null 2>&1; then
            print_status "OK" "Servidor responde correctamente"
        else
            print_status "WARN" "Servidor no responde"
            echo "   Asegúrate de que el backend de Fomalhaut esté ejecutándose"
        fi
    else
        print_status "INFO" "curl no disponible - saltando verificación de conectividad"
    fi
else
    print_status "ERROR" "config.json no encontrado"
fi
echo ""

# 6. Mostrar opciones de ejecución
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "6️⃣  Opciones de ejecución"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Para ejecutar el bridge con ESP32 real:"
echo "  python3 esp32_to_fomalhaut_bridge.py"
echo ""
echo "Para probar sin hardware (simulación):"
echo "  python3 simulate_esp32.py"
echo ""
echo "Para ver más información:"
echo "  cat README.md"
echo "  cat ARCHITECTURE.md"
echo "  cat BACKEND_SPEC.md"
echo ""

# 7. Resumen
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Validación completada"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Preguntar si desea ejecutar el bridge
read -p "¿Deseas ejecutar el bridge ahora? (y/N): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    print_status "INFO" "Iniciando bridge..."
    echo ""
    python3 esp32_to_fomalhaut_bridge.py
fi
