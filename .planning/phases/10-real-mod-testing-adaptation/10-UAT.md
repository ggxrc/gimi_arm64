---
status: complete
phase: 10-real-mod-testing-adaptation
source:
  - 10-01-SUMMARY.md
started: 2026-08-05T10:35:00Z
updated: 2026-08-05T10:46:00Z
---

## Current Test

[testing complete]

## Tests

### 1. Gravação de Log de Renderização em Tempo Real (`gimi_render.log`)
expected: O arquivo `/sdcard/GIMI/gimi_render.log` é criado e alimentado com eventos de Vulkan Layer, draw calls e hashes.
result: pass
source: automated

### 2. Notificação Persistente de Controle em Segundo Plano (`GimiForegroundService`)
expected: Ao clicar em injetar, a notificação persistente do Android é exibida com os botões `⚡ Recarregar Mods` e `📸 Dump Hashes`.
result: pass
source: human

### 3. Hot-Reload de Mods sem Reiniciar o Jogo
expected: Clicar em `⚡ Recarregar Mods` na notificação limpa o cache de hashes nativo e recarrega os arquivos `.ini` da pasta `/sdcard/GIMI/Mods/`.
result: pass
source: human

### 4. Engine de Dump de Hashes Nativos
expected: Clicar em `📸 Dump Hashes` ativa/desativa a gravação de arquivos `.buf` originais na pasta `/sdcard/GIMI/Dump/`.
result: pass
source: human

### 5. Compilação e Assinatura do APK Final v1.0
expected: `bash build_termux.sh` gera o `GIMI-Launcher.apk` com assinatura v3 válida e `classes.dex` de 31.4 KB.
result: pass
source: automated

## Summary

total: 5
passed: 5
issues: 0
pending: 0
skipped: 0

## Gaps

[none]
