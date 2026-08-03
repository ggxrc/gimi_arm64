# Phase 2: Parser de Configurações 3dmigoto & Gestão de Hashes - Context

**Gathered:** 2026-08-03
**Status:** Ready for planning

<domain>
## Phase Boundary

Esta fase implementa a "inteligência" do gimi_arm64 sobre a infraestrutura de interceptação da Fase 1. Entrega dois subsistemas independentes mas mutuamente dependentes:

1. **IniParser** — carregamento e interpretação dos arquivos `.ini` do 3dmigoto (Orfix.ini, Txfix.ini e overrides customizados) em estruturas de dados C++ tipadas.
2. **ResourceHashEngine** — cálculo rápido e cache de hashes de Vertex Buffers, Index Buffers e Texturas durante o render loop sem causar stutter.

O boundary desta fase termina na **leitura e indexação** dos dados — a *aplicação* das regras (troca de buffers, substituição de texturas) é responsabilidade da Fase 3 e 4.

</domain>

<decisions>
## Implementation Decisions

### Parser de Configurações
- **D-01:** Utilizar `inih` (C header-only, `ini.h`) como base de parsing de baixo nível. Envelopar em uma classe C++ `IniParser` com API tipada. `inih` é zero-dependency e compila limpo no NDK. — **Reversibility:** low — trocar a biblioteca base afeta apenas o layer de parsing interno.
- **D-02:** Estruturas de dados `ModDatabase` representando o conjunto carregado de regras: `TextureOverrideRule`, `ShaderOverrideRule`, `ResourceDeclaration`, `ConstantRule`. Indexadas por hash como chave primária para lookups O(1) durante o render.
- **D-03:** Suporte à diretiva `$include <filename>` para carregar Txfix.ini e outros módulos auxiliares a partir do arquivo raiz. Carregamento lazy na inicialização da engine.
- **D-04:** `ModDatabase` usa `std::unordered_map<uint32_t, Rule>` internamente (hash de 32 bits como chave), compatível com o formato de 8 hex dígitos do 3dmigoto original.

### Hash Engine
- **D-05:** Algoritmo primário: **xxHash64** (header-only, zero dependência, ~10 GB/s em ARM64). Hash de apenas os primeiros **4096 bytes** do buffer (configurável) para garantir zero stutter no render loop. — **Reversibility:** costly — mudar o algoritmo invalida todo o cache de hashes dos mods existentes.
- **D-06:** `HashRegistry` usa `VkBuffer` / `VkImage` (handle opaco) como chave de lookup: ponteiro-como-inteiro para acesso O(1). Cache invalidado nos eventos `vkDestroyBuffer` / `vkDestroyImage` interceptados.
- **D-07:** `std::shared_mutex` para leitura concorrente thread-safe (múltiplas draw calls lendo simultaneamente em engines multi-threaded). Escritas (novas entradas de hash) protegidas por lock exclusivo.

### Agent's Discretion
- Estrutura interna dos arquivos de cabeçalho e divisão dos `.cpp`.
- Estratégia de normalização de chaves do `.ini` (case-insensitive, trim de espaços).
- Tamanho exato do sample buffer (padrão 4096 bytes, ajustável via constante).

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project & System Architecture
- `.planning/PROJECT.md` — Visão geral e premissas anti-cheat do gimi_arm64
- `.planning/REQUIREMENTS.md` — Requisitos CFG-01, CFG-02, CFG-03
- `.planning/research/STACK.md` — Stack recomendada (inih, xxHash)
- `.planning/research/ARCHITECTURE.md` — Diagrama de componentes: Config Engine no layer 2
- `.planning/research/PITFALLS.md` — Avisos sobre stutter de hashing e edge cases de parsing

### Phase 1 Output (Base Layer)
- `include/graphics/vulkan_layer.h` — InstanceDispatchTable / DeviceDispatchTable onde os hooks de buffer serão registrados
- `include/graphics/graphics_dispatcher.h` — GraphicsDispatcher que acionará o HashEngine por fase
- `include/utils/logger.h` — Macros de logging disponíveis

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `include/utils/logger.h` — LOGD/LOGI/LOGW/LOGE já disponíveis
- `include/graphics/vulkan_layer.h` — DeviceDispatchTable com hooks de `vkCmdDrawIndexed` (Phase 3 consumirá os resultados desta fase)

### Integration Points
- O `ResourceHashEngine` será chamado dentro dos interceptors Vulkan da Fase 1 (dentro de `vkCmdBindVertexBuffers` / `vkCmdBindIndexBuffer`).
- A `ModDatabase` será consultada pelo Resource Replacement Engine da Fase 3.

</code_context>

<specifics>
## Specific Implementation Notes

- O parser deve produzir um `ModDatabase` totalmente em memória — sem leituras síncronas de arquivo durante frames de render (pré-carregamento na inicialização).
- A saída do HashEngine é um `uint64_t` internamente; ao fazer lookup no `ModDatabase`, deve-se truncar para `uint32_t` (formato 3dmigoto) para compatibilidade com mods existentes.
- Arquivos `.ini` residem no diretório de mods configurável (ex: `/sdcard/gimi_mods/`) — o caminho base será parametrizado via constante de compilação ou config.

</specifics>

<deferred>
## Deferred Ideas

- Hot-reloading de arquivos `.ini` (monitorar `inotify` em background) — adiado para v2
- Parser de `[KeySwap]` e lógica condicional complexa (`if $active == 1`) — adiado para v2
- Suporte a hashing de shaders SPIR-V (hashar bytecode de shaders) — Fase 3

</deferred>

---

*Phase: 2-Parser de Configurações 3dmigoto & Gestão de Hashes*
*Context gathered: 2026-08-03*
