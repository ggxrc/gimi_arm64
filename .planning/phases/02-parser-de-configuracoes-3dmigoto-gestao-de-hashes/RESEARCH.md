# Phase 2: Parser de Configurações 3dmigoto & Gestão de Hashes - Research

**Researched:** 2026-08-03
**Phase:** 02-parser-de-configuracoes-3dmigoto-gestao-de-hashes
**Depends on:** Phase 1 (Engine de Interceptação Vulkan ARM64)

## Executive Summary

A Fase 2 implementa os dois sistemas de dados centrais do gimi_arm64: o **parser de arquivos `.ini` do 3dmigoto** (que carrega as regras de substituição de mods como Orfix.ini e Txfix.ini) e o **engine de cálculo e cache de hashes de recursos gráficos** (que identifica buffers e texturas em tempo real sem introduzir latência perceptível no render loop). Estes dois sistemas formam a "inteligência" do hook — sem eles, a camada de interceptação da Fase 1 sabe interceptar chamadas, mas não sabe *o que* substituir.

---

## Domínio: Sintaxe do 3dmigoto `.ini`

### Seções Suportadas (MVP)
| Seção | Propósito |
|-------|-----------|
| `[TextureOverride.*]` | Substituição de texturas por hash |
| `[ShaderOverride.*]` | Substituição/patch de shaders por hash |
| `[Resource.*]` | Declaração de recursos externos (arquivos de buffer/textura) |
| `[Constants]` | Variáveis globais e condições lógicas |
| `[KeySwap]` / `[KeyOverride]` | Grupos de substituição condicionais (deferred to v2) |

### Formato Representativo

```ini
; Orfix.ini — correção de iluminação para personagem Lumine
[TextureOverride.LumineFace]
hash = a1b2c3d4
ps-t0 = ResourceLumineFaceNew

[Resource.ResourceLumineFaceNew]
filename = LumineFaceNew.dds

[ShaderOverride.ShadowFix]
hash = deadbeef
ps = ResourcePatchedShader

[Constants]
global $active = 1
```

### Estratégia de Parsing
- **Biblioteca base**: `inih` (C, header-only) — compatível com NDK, zero dependências externas.
- **Normalização de chaves**: chaves são case-insensitive, comentários iniciados por `;` ou `#`.
- **Estruturas C++**: `IniSection` → `std::unordered_map<std::string, std::string>` de pares key=value.
- **Multi-file**: suporte a `include` de arquivos auxiliares (Txfix.ini, etc.) via diretiva `$include`.

---

## Domínio: Hashing de Recursos Gráficos

### O que precisa ser hasheado
| Recurso | Quando hashar | Algoritmo |
|---------|--------------|-----------|
| Vertex Buffer | Em `vkCmdBindVertexBuffers` — primeiros N bytes do buffer | xxHash64 ou CRC32C |
| Index Buffer | Em `vkCmdBindIndexBuffer` | xxHash64 ou CRC32C |
| Textura (descriptor) | Em `vkUpdateDescriptorSets` — usar dimensões + format como fingerprint | xxHash64 |

### Estratégia de Cache
- **Chave de cache**: ponteiro do `VkBuffer` / `VkImage` como handle de identidade (stable dentro do mesmo frame).
- **Invalidação**: cache é invalidado quando o buffer é destruído (`vkDestroyBuffer` / `vkDestroyImage`).
- **Thread safety**: `std::shared_mutex` para read-heavy access (muitas draw calls lendo, poucas escritas).
- **Anti-stutter**: hashing limitado a no máximo N bytes (configurável, padrão 4 KB) para evitar latência em buffers grandes.

### Algoritmo Recomendado: xxHash64 / CRC32C
- **xxHash64**: extremamente rápido (~10 GB/s em ARM), API de 2 funções, header-only, zero alocações. Preferido para Phase 2.
- **CRC32C**: suportado em instrução nativa ARM (`crc32` em ARMv8.1+), mas menos portável. Reserva para fallback.

---

## Integração com a Fase 1

```
vkCmdBindVertexBuffers  ──→  ResourceHashEngine::compute(buffer)
                                     │
                              cache hit?  ──yes──→  HashRegistry::lookup(hash)
                                     │                      │
                                    no                  ModDatabase::find(hash)
                                     │                      │
                              xxHash64 compute         Rule found → Phase 3 swap
                                     │
                              HashRegistry::store(buffer, hash)
```

---

## Pitfalls Relevantes para esta Fase

- **Stutter em hashing síncrono**: Nunca hashar o buffer inteiro dentro do draw call. Usar amostragem dos primeiros N bytes + dimensões.
- **Collisions de hash**: IDs de mods do 3dmigoto são CRC32 de 8 hex dígitos. Nosso sistema deve armazenar ambos (hash interno + hash do mod) para matching correto.
- **Syntax edge cases do `.ini`**: Orfix.ini pode ter valores multi-linha com `\` e seções com nomes duplicados. O parser deve tolerar isso graciosamente.
- **Encoding**: Arquivos `.ini` de mods podem estar em UTF-8 ou Latin-1. Normalizar para UTF-8 no load.
