#!/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
# gimi_arm64 — Native Termux Compilation Script (No SDK Licenses Required)
# ─────────────────────────────────────────────────────────────────────────────

set -e

echo "=== Compilando gimi_arm64 para Android ARM64 ==="

mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

echo ""
echo "✅ Compilação concluída com sucesso!"
echo "Biblioteca gerada: build/libgimi_arm64.so"
echo "Executável de teste: build/test_launcher_integration"
