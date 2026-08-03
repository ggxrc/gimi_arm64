# Phase 1: Engine de Interceptação Vulkan ARM64 & Infraestrutura Nativa - Context

**Gathered:** 2026-08-03
**Status:** Ready for planning

<domain>
## Phase Boundary

Esta fase estabelece o ambiente de compilação Android NDK C++20 para `arm64-v8a` e a camada de interceptação não-destrutiva de renderização gráfica em memória (suportando Vulkan e OpenGL ES dinamicamente). Ela entrega a infraestrutura base do hook sem modificar o APK ou arquivos do jogo no disco.

</domain>

<decisions>
## Implementation Decisions

### Graphics Interception Architecture
- **D-01:** Suporte dinâmico a Dual-API (Vulkan e OpenGL ES) em tempo de execução. O dispatcher de hooking irá interceptar entrypoints de ambas as APIs (`vkGetInstanceProcAddr`/`vkGetDeviceProcAddr` e `eglGetProcAddress`) e ativar o interceptador correspondente à API gráfica inicializada pelo jogo. — **Reversibility:** costly — mudar a estrutura de dispatch afeta todas as chamadas de render downstream.
- **D-02:** Interceptação 100% não-destrutiva executada exclusivamente em memória via Vulkan Layer / EGL Dispatch Hooking sem alterar o APK original, binaries no disco ou verificações de SHA256 para prevenir banimentos pelo anti-cheat. — **Reversibility:** one-way — violação da premissa não-destrutiva introduz risco direto de detecção por anti-cheat.

### Toolchain & Native Build Infrastructure
- **D-03:** Toolchain nativa Android NDK r26b+ com C++20 direcionada exclusivamente para a arquitetura `arm64-v8a` via CMake target `libgimi_arm64.so`.
- **D-04:** Sistema de logging nativo integrado ao `android/log.h` (tag `gimi_arm64`) com macros de nivelamento configuráveis (DEBUG, INFO, ERROR).

### the agent's Discretion
- Organização interna dos diretórios do CMake/C++ nativo.
- Seleção de bibliotecas utilitárias leves para rotinas nativas (ex: Dobby / Substrate symbol resolver).

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project & System Architecture
- `.planning/PROJECT.md` — Visão geral do gimi_arm64, restrições e premissas de segurança anti-cheat
- `.planning/REQUIREMENTS.md` — Requisitos funcionais da camada de injeção e hooking (HOOK-01, HOOK-02, HOOK-03)
- `.planning/research/STACK.md` — Recomendações de stack NDK, Vulkan Layer spec e Dobby
- `.planning/research/ARCHITECTURE.md` — Diagrama de componentes e fluxo do dispatcher de gráficos
- `.planning/research/PITFALLS.md` — Alertas sobre detecção de anti-cheat e mitigação não-destrutiva

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `AGENTS.md`: Guias do projeto e convenções GSD

### Integration Points
- Entrypoints da `libvulkan.so` e `libEGL.so` / `libGLESv3.so` no ambiente Android ARM64

</code_context>

<specifics>
## Specific Ideas

- Suporte transparente a aparelhos que rodam Genshin Impact via Vulkan ou via OpenGL ES através da detecção automática na inicialização da engine nativa.

</specifics>

<deferred>
## Deferred Ideas

- Overlay in-game com ImGui (adiado para v2)
- Hot-reloading de arquivos `.ini` (adiado para v2)

</deferred>

---

*Phase: 1-Engine de Interceptação Vulkan ARM64 & Infraestrutura Nativa*
*Context gathered: 2026-08-03*
