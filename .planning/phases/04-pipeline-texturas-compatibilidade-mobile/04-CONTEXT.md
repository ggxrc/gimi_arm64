# Phase 4 Context: Pipeline de Texturas & Compatibilidade de Formatos Móveis

## Phase Goal
Interceptar chamadas de Vulkan referentes a criação e bind de texturas, calcular hashes do conteúdo dos pixels para identificação, e substituir de maneira não-destrutiva as texturas originais do jogo pelas texturas fornecidas pelos mods em formatos amigáveis para mobile (ASTC/ETC2).

## Requirements Addressed
- **TEX-01**: Interceptar descritores e uploads de texturas na memória VRAM do Vulkan.
- **TEX-02**: Suportar carregamento de texturas nos formatos móveis nativos (ASTC / ETC2 / RGBA8).
- **TEX-03**: Substituir bind de texturas originais do jogo pelas texturas do mod quando os hashes corresponderem.

## Architecture
- **TextureHasher**: Integrado no hook de `vkCmdCopyBufferToImage` para gerar hash do conteúdo bruto que está sendo transferido para a VRAM.
- **ImageTracker**: Mapeia `VkImage` → `hash` e `VkImageView` → `VkImage` para podermos identificar uma textura no momento em que ela é anexada a um shader via Descriptor Set.
- **TextureSwapper / TextureLoader**: Carrega os arquivos de textura dos mods, cria os respectivos objetos `VkImage` e `VkImageView` (mantendo-os alocados na VRAM) e intercepta `vkUpdateDescriptorSets` para trocar o ponteiro do `VkImageView` original pelo do mod.

## Execution Constraints
- A substituição deve ocorrer no momento do bind no Descriptor Set, e não através da cópia por cima dos bytes originais do jogo (para preservar o arquivo/memória original).
- Formatos de PC (DDS/BC7) não são suportados nativamente por todas as GPUs mobile. Os mods já deverão vir em formatos mobile (ASTC/ETC2) ou RGBA8 bruto. O motor apenas aloca e submete o buffer.
