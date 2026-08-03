# Feature Landscape: gimi_arm64

**Domain:** Graphics Hooking & Model Importing for Android ARM64
**Researched:** 2026-08-03

## Table Stakes

Features expected for functional 3dmigoto mod importing on mobile. Missing = mods won't render correctly.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Interceptação de Render (Vulkan/GLES) | Necessário para capturar e substituir chamadas de desenho em tempo de execução | Alta | Interceptação via Vulkan layer ou EGL hook |
| Parser de Configuração `.ini` do 3dmigoto | Compatibilidade com a sintaxe de mods existentes (seções `[TextureOverride]`, `[ShaderOverride]`, etc.) | Média | Suporte completo a Orfix.ini, Txfix.ini e regras customizadas |
| Substituição de Vertices e Indices (Mesh Swap) | Permite trocar modelos 3D originais por modelos customizados | Alta | Mapeamento de vertex buffers em ARM64 |
| Substituição de Texturas (Texture Swap) | Permite carregar novas texturas em tempo de execução | Média | Conversão/re-upload para memória GPU em formato móvel |
| Interceptação Não-Destrutiva | Evita alterar arquivos originais do jogo no disco para proteção contra banimentos | Alta | Interceptação puramente em nível de API de gráficos |

## Differentiators

Recursos adicionais que diferenciam o gimi_arm64 e melhoram a experiência no mobile.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Conversão de Textura On-the-Fly | Transforma texturas DDS (BC1-BC7) em ASTC/ETC2 automaticamente | Alta | Evita necessidade de re-converter manualmente todos os mods de PC |
| Overlay ImGui / UI no Jogo | Permite ativar/desativar mods e alternar presets diretamente no jogo | Média | Renderizado como uma camada Vulkan sobreposta |
| Hot-Reloading de Mods | Recarrega arquivos `.ini` e recursos sem precisar fechar o jogo | Média | Facilita testes e criação de mods |

## Anti-Features

Recursos e abordagens que devem ser estritamente EVITADOS.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| Modificação de APK / Arquivos de Asset no Disco | Causa inconsistência nos checadores de integridade do anti-cheat, resultando em banimento | Usar exclusivamente hooking dinâmico em memória via Vulkan Layer / EGL |
| Modificação de Código Executável do Jogo | Alterar bytes do executável principal ativa verificações de memória do anti-cheat | Limitar o escopo de hooking aos entrypoints da biblioteca gráfica |

## Feature Dependencies

```
Interceptação de Render → Parser de .ini → Substituição de Texturas/Meshes → Hot-Reloading / UI Overlay
```

## MVP Recommendation

Prioritize:
1. Interceptação estável de Vulkan Draw Calls em ARM64
2. Parser funcional para `.ini` do 3dmigoto (Orfix.ini & Txfix)
3. Substituição não-destrutiva de Vertices, Indices e Texturas

Defer: Overlay visual no jogo e Hot-reloading avançado para fases posteriores.
