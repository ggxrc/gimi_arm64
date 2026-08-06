# gimi_arm64

**Importador de Modelos 3dmigoto para Android ARM64 — Interceptador Gráfico Não-Destrutivo Vulkan & OpenGL ES**

O `gimi_arm64` é um port móvel do Genshin Impact Model Importer (GIMI, baseado no 3dmigoto) para a arquitetura ARM64, focado especificamente no Android. Ele permite a substituição de modelos 3D, texturas e correções de shaders (como as regras do `Orfix.ini` e `Txfix.ini`) diretamente no pipeline gráfico do jogo.

Todo esse processo é feito de forma **não-destrutiva**: não há modificação de APK, não é necessário acesso root e nenhuma alteração é feita nos arquivos do jogo no disco, garantindo a segurança contra sistemas anti-cheat.

---

## Como Funciona (Mecanismo Técnico)

O mecanismo de interceptação gráfica é transparente e opera inteiramente em memória (RAM).

```
Genshin Impact (Processo Android ARM64)
         │
         ▼  Chamadas de API Vulkan / GLES
┌────────────────────────────────────────┐
│ libgimi_arm64.so                       │
│ (Vulkan Layer / Tabela de Despacho EGL)│
│ ────────────────────────────────────── │
│  - Camada Vulkan interceptando draws   │  ← Intercepta vkCmdDrawIndexed
│  - Hook EGL (redirecionamento Dobby)   │  ← Redireciona entrypoints do OpenGL ES
└────────────────────────────────────────┘
         │
         ▼  Correspondência de Hash & Substituição
┌────────────────────────────────────────┐
│ ModManagerService & MeshSwapper        │
│  - Mods do 3dmigoto (.ini) processados │  ← Lê de /sdcard/GIMI/Mods/
│  - Substituição de Vertex/Index Buffer │  ← Remapeia stride & atributos
└────────────────────────────────────────┘
         │
         ▼
    Hardware GPU
```

### 1. Injeção de Camada Vulkan
No Vulkan, o aplicativo é injetado usando o mecanismo de camada implícita do carregador do sistema. O sistema Android ativa camadas de depuração se configurado em `Settings.Global`. O launcher coordena com o Android através de secure settings para especificar:
- `enable_gpu_debug_layers = 1`
- `gpu_debug_app = <pacote_do_jogo>`
- `gpu_debug_layer_app = com.gimi.launcher`
- `gpu_debug_layers = VK_LAYER_GIMI_ARM64`

Isso faz com que o carregador Vulkan do Android encadeie automaticamente nossa biblioteca de camadas (`libgimi_arm64.so`) dentro da instância Vulkan do jogo, permitindo-nos interceptar chamadas de desenho (ex: `vkCmdDrawIndexed` e `vkCmdBindVertexBuffers`) e trocar recursos quando seus resumos xxHash64 correspondem a um mod.

### 2. Redirecionamento OpenGL ES / EGL
Para a interceptação do GLES, a `libgimi_arm64.so` intercepta o `eglGetProcAddress` usando a biblioteca Dobby. Isso intercepta dinamicamente os ponteiros de retorno para funções de desenho do OpenGL, como `glDrawElements` e atualizações de buffer, trocando as associações de buffer na memória quando os recursos alvo correspondem.

---

## Recursos

- **Sem Necessidade de Root**: Funciona completamente em espaço de usuário.
- **Interceptação Vulkan & GLES**: Suporta totalmente os pipelines de renderização Vulkan 1.3 e OpenGL ES 3.2.
- **Compatibilidade com Configurações do 3dmigoto**: Analisa e aplica substituições de seções `.ini` (vertex buffers, index buffers, substituições de shader, etc.).
- **Correções de Shader**: Executa patches de shader SPIR-V e manipula ajustes de layout para corrigir problemas de sombras e transparência (regras `Orfix`/`Txfix`).
- **Pipeline de Texturas Móvel**: Suporta o carregamento e substituição transparente de formatos de textura nativos de dispositivos móveis (ASTC, ETC2, RGBA8).
- **Interface de Gerenciamento Dinâmico**: Busque, filtre e ative/desative mods individuais dinamicamente através de um aplicativo launcher moderno com tema escuro premium.

---

## Configuração & Pré-requisitos

Como o `gimi_arm64` atualiza as configurações seguras do sistema para injetar a camada gráfica, ele requer a permissão do Android `WRITE_SECURE_SETTINGS`.

### Passo 1: Instalar o Shizuku
Para conceder permissões sem um PC, é recomendado usar o **Shizuku**:
1. Baixe o Shizuku na Google Play Store ou no GitHub.
2. Abra o Shizuku e siga as instruções para iniciá-lo via **Depuração por Rede Sem Fio** (sem necessidade de root).
3. Com o Shizuku rodando, abra o **GIMI Launcher** e clique no botão **"Conceder Permissão Auto via Shizuku"**.

### Passo 2: Alternativa (PC via ADB)
Se preferir usar um PC, conecte seu dispositivo via USB com a Depuração USB ativada e execute o seguinte comando:
```bash
adb shell pm grant com.gimi.launcher android.permission.WRITE_SECURE_SETTINGS
```

---

## Como Instalar e Gerenciar Mods

1. Crie um diretório chamado `/sdcard/GIMI/Mods/` no seu dispositivo.
2. Extraia as pastas de mods do 3dmigoto dentro de `/sdcard/GIMI/Mods/`. Cada mod deve estar em sua própria subpasta contendo a configuração `.ini` e os assets `.buf`/`.dds`.
3. Abra o **GIMI Launcher**.
4. Navegue até a aba **Mod Manager** na parte inferior para pesquisar, verificar a contagem de arquivos ini e ativar/desativar mods individualmente.
5. Volte para a aba **Dashboard**, selecione a variante do jogo instalada (Global/Play Store, Samsung, China) e clique em **"INJECT LAYER & LAUNCH GAME"**.

---

## Compilar a partir do Código Fonte

### Usando Gradle
Compile o projeto usando o Gradle padrão do Android:
```bash
./gradlew assembleDebug
```

### Compilação Leve (Termux / Script NDK)
Se for compilar diretamente no dispositivo Android via Termux ou em um ambiente de terminal leve:
```bash
bash build_termux.sh
```
Este script compila os alvos nativos C++ (`libgimi_arm64.so`), compila as classes Java/Kotlin, converte para bytecode dex, empacota e assina o APK final `GIMI-Launcher.apk`.

---

## Guia de Contribuição

Contribuições para melhorar a estabilidade da interceptação, suporte de arquivos de configuração ou a interface gráfica são bem-vindas.

### Diretrizes:
- **C++20 Limpo**: O código nativo deve ser escrito usando C++20 moderno com tabelas de despacho padrão e padrões RAII.
- **Segurança em Primeiro Lugar**: Nunca introduza patches de memória que modifiquem segmentos de código do executável do jogo. Intercepte apenas nos limites do driver gráfico (Camada Vulkan / EGL Dispatch).
- **Commits Organizados**: Siga a convenção de mensagens de commit (ex: `feat(subsystem): description`, `fix(subsystem): fix bug`).

---

## Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo LICENSE para mais detalhes.
