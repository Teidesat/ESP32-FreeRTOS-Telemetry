#!/bin/bash
# Script rápido para iniciar el bridge ESP32 → Fomalhaut

echo "🌉 Iniciando ESP32 → Fomalhaut Bridge..."
echo ""

# Verificar que Python3 está instalado
if ! command -v python3 &> /dev/null; then
    echo "❌ Error: Python3 no está instalado"
    echo "💡 Instala Python3: sudo apt install python3 python3-pip"
    exit 1
fi

# Verificar que las dependencias están instaladas
if ! python3 -c "import serial" &> /dev/null; then
    echo "⚠️  Dependencias no instaladas. Instalando..."
    pip install -r requirements.txt
    echo ""
fi

# Ejecutar el bridge
python3 bridge.py

echo ""
echo "✅ Bridge finalizado"
