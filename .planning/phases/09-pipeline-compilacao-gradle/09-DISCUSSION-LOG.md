# Discussion Log - Phase 9: Pipeline de Compilação Padrão Gradle & Reestruturação da Codebase

## Selected Discussion Areas

1. **Estrutura de Diretórios e Módulos**
   - **Opção Escolhida:** Mover `android/app` para `app/` na raiz do repositório, mantendo `src/` e `include/` para C++ nativo na raiz.
   - **Motivação:** Alinha a estrutura ao padrão Android Studio (`:app`), mantendo o código C++ organizado na raiz do projeto.

2. **Migração e Limpeza da UI**
   - **Opção Escolhida:** Migrar 100% para Kotlin + Jetpack Compose e incluir o compilador Kotlin na pipeline de build.
   - **Ações de Limpeza:**
     - Remover arquivos `.java` legados (`MainActivity.java`, `GimiNativeBridgeJava.java`, `ModInfoJava.java`).
     - Limpar stubs C++ legados em `shizuku_layer_injector.cpp`.
     - Criar `res/values/strings.xml` e `res/values/colors.xml` para centralizar recursos da interface.
