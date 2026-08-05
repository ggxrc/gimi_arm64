# Phase 10: Testes Reais, Adaptação de Mods & Notificação de Controle - Context

## Locked Decisions

### 1. Critério Estrito de Conclusão da Fase 10 (v1.0 Final)
- **Conclusão:** A Fase 10 (e a versão v1.0) só será finalizada quando o mod real do 3dmigoto (Mod da Paimon em `/sdcard/GIMI/Mods/Paimon/` ou mod selecionado) for renderizado com sucesso no jogo no dispositivo real.

### 2. Log de Renderização em Tempo Real (`gimi_render.log`)
- **Arquivo de Log:** `/sdcard/GIMI/gimi_render.log`
- **Conteúdo dos Logs:**
  - Inicialização e carregamento da Vulkan Layer (`libgimi_arm64.so`).
  - Log detalhado de Draw Calls interceptadas (`vkCmdDrawIndexed` / `glDrawElements`).
  - Hashes calculados de Vertex Buffers, Index Buffers e Texturas.
  - Matches com seções `[TextureOverride...]` e `[ShaderOverride...]` dos arquivos `.ini`.
  - Trocas de buffers/texturas efetuadas com sucesso e erros de substituição.

### 3. Notificação Fixa Persistente (Hot-Reload & Dump Manager)
- **Serviço de Notificação Android:** `GimiForegroundService` com notificação persistente na barra do Android.
- **Ações na Notificação:**
  - **`⚡ Recarregar Mods`**: Envia comando para a engine nativa recarregar as configurações `.ini` e texturas sem reiniciar o jogo.
  - **`📸 Dump Hashes`**: Ativa/desativa a extração automática dos hashes de modelos/texturas do jogo em tempo real.

### 4. Engine Nativa de Dump de Hashes (C++)
- **Diretório de Output:** `/sdcard/GIMI/Dump/`
- **Funcionamento:** Quando o modo Dump está ativado via notificação/app, a engine grava os Vertex Buffers originais, Index Buffers e Texturas capturadas em disco com seus respectivos nomes de hash (ex: `0x1f7f0600.buf`, `0x89ab1234.dds`).

---

## Technical Tasks for Phase 10 Execution

1. **Notificação Fixa Persistente (`GimiForegroundService.java`)**:
   - Implementar `GimiForegroundService` no pacote `com.gimi.launcher.service` com ações de Intent para `ACTION_HOT_RELOAD` e `ACTION_TOGGLE_DUMP`.
   - Adicionar permissão `FOREGROUND_SERVICE` no `AndroidManifest.xml`.
2. **Logs de Renderização em Arquivo (`logger.cpp` / `logger.h`)**:
   - Atualizar a engine de logging C++ para gravar mensagens formatadas em `/sdcard/GIMI/gimi_render.log` de forma assíncrona/segura.
3. **Mecanismo de Dump de Hashes C++ (`resource_hash_engine.cpp`)**:
   - Adicionar suporte a salvamento de buffers/texturas originais no disco em `/sdcard/GIMI/Dump/` quando `dump_enabled == true`.
4. **Validação E2E no Dispositivo**:
   - Testar injeção do mod da Paimon em `/sdcard/GIMI/Mods/Paimon/` no jogo e confirmar visualmente a renderização do modelo modificado.
