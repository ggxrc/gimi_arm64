# Domain Pitfalls: gimi_arm64

**Domain:** Android Vulkan Hooking & 3dmigoto Porting
**Researched:** 2026-08-03

## Critical Pitfalls

### Pitfall 1: Detecção por Sistemas Anti-Cheat (Hoyoverse Anti-Cheat)
**What goes wrong:** Modificação de arquivos do jogo (APK, .so do jogo) ou injeção agressiva na memória de código ativa varreduras de integridade do anti-cheat, resultando em banimento da conta.
**Why it happens:** Anti-cheats no Android checam assinaturas de pacotes, somas de verificação SHA256 dos binaries no disco e regiões de memória de código.
**Consequences:** Banimento permanente da conta do jogo.
**Prevention:** Utilizar exclusivamente interceptação limpa em nível de Vulkan Layer / EGL Dispatch Table sem modificar arquivos no disco nem memórias do processo principal do jogo.

### Pitfall 2: Incompatibilidade de Formatos de Textura (DDS BC7 vs Mobile ASTC)
**What goes wrong:** Texturas de mods criadas para PC utilizam compressão DirectX (DDS BC1/BC3/BC7), que não são suportadas nativamente por GPUs ARM Mali ou Qualcomm Adreno.
**Why it happens:** GPUs mobile usam compressores como ASTC e ETC2.
**Consequences:** Texturas pretas, corrompidas ou travamento de renderização.
**Prevention:** Implementar módulo de conversão ou suporte a ASTC no pipeline de carregamento do gimi_arm64.

## Moderate Pitfalls

### Pitfall 1: Diferenças de Stride e Alignment de Vertex Buffers (DX11 vs Vulkan ARM64)
**What goes wrong:** Atributos de vértices de 3dmigoto de PC podem usar alinhamentos de memória e posições de stream específicos do DirectX 11.
**Prevention:** Criar uma camada de remapeamento de `VkVertexInputAttributeDescription` e `VkVertexInputBindingDescription` para traduzir o layout de entrada dos mods.

## Phase-Specific Warnings

| Phase Topic | Likely Pitfall | Mitigation |
|-------------|---------------|------------|
| Vulkan Hooking | Interceptação incorreta de `vkCmdDrawIndexed` travando a GPU | Validar com Vulkan Validation Layers durante o desenvolvimento |
| .ini Parsing | Falha ao interpretar sintaxe específica de `Orfix.ini` | Testar parser com suíte de testes unitários baseada em configs reais |
