# Requirements: gimi_arm64

**Defined:** 2026-08-03
**Core Value:** Interceptar e substituir recursos gráficos (modelos, texturas e shaders) de forma não-destrutiva via camada de gráficos Vulkan/GLES em ARM64 Android, mantendo compatibilidade com os mods e fixes (.ini) do 3dmigoto.

## v1 Requirements

Requirements for initial release. Each maps to roadmap phases.

### Interception & Engine Infrastructure

- [ ] **HOOK-01**: Configurar compilação nativa Android NDK C++20 direcionada para a arquitetura `arm64-v8a`
- [ ] **HOOK-02**: Implementar estrutura Vulkan Layer / EGL Dispatch Table para interceptar entrypoints gráficos nativos (`vkGetInstanceProcAddr`, `vkGetDeviceProcAddr`)
- [ ] **HOOK-03**: Interceptar chamadas de renderização de forma puramente em memória (sem modificar arquivos de jogo ou APK no disco)

### Config Parsing & Hashes

- [ ] **CFG-01**: Desenvolver parser C++ de arquivos `.ini` do 3dmigoto capaz de ler seções `[TextureOverride]`, `[ShaderOverride]`, `[Resource]` e `[Constants]`
- [ ] **CFG-02**: Implementar sistema de geração e cache de hash para Vertex Buffers, Index Buffers e Texturas em tempo de execução
- [ ] **CFG-03**: Suportar carregamento de regras globais e condicionais presentes em arquivos de mod padrão

### Mesh & Model Overrides

- [ ] **MESH-01**: Interceptar chamadas de desenho (`vkCmdDrawIndexed` / `vkCmdBindVertexBuffers`) e substituir Vertex Buffers por modelos modded
- [ ] **MESH-02**: Realizar remapeamento de atributos de vértices (stride e layout) de DX11 para o formato de input do Vulkan

### Shader Fixes & Lighting

- [ ] **FIX-01**: Implementar suporte ao parser e regras de execução do `Orfix.ini` para correção de iluminação e sombras
- [ ] **FIX-02**: Implementar suporte ao parser e regras de execução do `Txfix.ini` para ajustes de transparência e coordenadas UV
- [ ] **FIX-03**: Substituir ou patchear SPIR-V / shaders em tempo de execução para aplicar correções de renderização

### Texture Pipeline

- [ ] **TEX-01**: Interceptar descritores e uploads de texturas na memória VRAM do Vulkan
- [ ] **TEX-02**: Suportar carregamento de texturas nos formatos móveis nativos (ASTC / ETC2 / RGBA8)
- [ ] **TEX-03**: Substituir bind de texturas originais do jogo pelas texturas do mod quando os hashes corresponderem

## v2 Requirements

Deferred to future release. Tracked but not in current roadmap.

### UI & Developer Experience

- **UI-01**: Implementar overlay in-game com ImGui (Vulkan) para ativar/desativar mods visualmente
- **UI-02**: Suportar Hot-Reloading de arquivos `.ini` e texturas sem reiniciar o jogo

## Out of Scope

Explicitly excluded. Documented to prevent scope creep and anti-cheat bans.

| Feature | Reason |
|---------|--------|
| Modificação de APK ou arquivos de asset no disco | Evitar inconsistências de verificação SHA256 do anti-cheat Hoyoverse que levariam ao banimento da conta |
| Modificação de memória do código executável do jogo | Manter a injeção isolada estritamente na camada de driver gráfico Vulkan/GLES |
| Suporte a iOS / ARM32 | Foco v1 exclusivamente em Android ARM64 (`arm64-v8a`) |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| HOOK-01 | TBD | Pending |
| HOOK-02 | TBD | Pending |
| HOOK-03 | TBD | Pending |
| CFG-01 | TBD | Pending |
| CFG-02 | TBD | Pending |
| CFG-03 | TBD | Pending |
| MESH-01 | TBD | Pending |
| MESH-02 | TBD | Pending |
| FIX-01 | TBD | Pending |
| FIX-02 | TBD | Pending |
| FIX-03 | TBD | Pending |
| TEX-01 | TBD | Pending |
| TEX-02 | TBD | Pending |
| TEX-03 | TBD | Pending |

**Coverage:**
- v1 requirements: 14 total
- Mapped to phases: 0
- Unmapped: 14 ⚠️

---
*Requirements defined: 2026-08-03*
*Last updated: 2026-08-03 after initial definition*
