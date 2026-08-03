# Phase 5 Context: Overlay In-Game com Dear ImGui & Hot-Reloading de Mods

## Phase Goal
Implementar uma interface de overlay *in-game* utilizando **Dear ImGui** (renderizada via Vulkan) para permitir ao usuário visualizar o status da injeção, ativar/desativar mods individualmente durante a execução do jogo, e recarregar arquivos de configuração (`.ini`) e recursos (texturas/meshes) via **Hot-Reloading** sem a necessidade de reiniciar a aplicação.

## Requirements Addressed
- **UI-01**: Implementar overlay in-game com ImGui (Vulkan) para ativar/desativar mods visualmente.
- **UI-02**: Suportar Hot-Reloading de arquivos `.ini` e texturas sem reiniciar o jogo.

## Key Architectural Decisions

1. **Dear ImGui Backend (Vulkan + Android Input)**:
   - Integrar ImGui com o backend `imgui_impl_vulkan.cpp` e interrupções de renderização no hook `vkQueuePresentKHR` ou no final da render pass.
   - Capturar eventos de toque (`MotionEvent`) do Android via hooking/interceptação nativa da `ANativeWindow` ou `input` handler.

2. **Hot-Reloading Engine**:
   - Adicionar um observador de sistema de arquivos (*file watcher* ou polling leve de `mtime`) na pasta `/sdcard/GIMI/Mods/`.
   - Ao detectar alteração em um arquivo `.ini`, reconstruir a tabela de regras na `ModDatabase` sem interromper o loop de renderização (com `std::shared_mutex` para leitura/escrita não-bloqueante).
   - Limpar caches no `HashRegistry` e disparar releitura no `TextureSwapper` e `MeshSwapper`.

3. **Overlay Control Panel**:
   - Menu flutuante compacto com alternadores de estado (*toggle switches*) para cada pasta de mod detectada.
   - Botão manual "Reload All Mods".
   - Painel de estatísticas de debug (número de draw calls interceptadas, texturas trocadas, hashes em cache).

## Execution Constraints
- O ImGui deve ser compilado nativamente no NDK sem causar sobrecarga relevante de framerate (< 0.5ms por frame).
- O hot-reloading deve garantir thread-safety total para evitar *data races* enquanto o pipeline Vulkan estiver gravando comandos.
