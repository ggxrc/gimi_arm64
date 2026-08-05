# Phase 9: Pipeline de Compilação Padrão Gradle (`gradlew assembleDebug`) - Research

## Executive Summary
Pesquisa para estruturação do ambiente de compilação oficial do Android Studio / Gradle CLI (`gradle assembleDebug`) no repositório `gimi_arm64`.

---

## 1. Estruturação do `settings.gradle` para a Raiz do Repositório
Para permitir que comandos invocados da raiz do repositório (ex: `gradle assembleDebug`) encontrem o módulo Android localizado em `android/app`, o `settings.gradle` precisa mapear explicitamente a propriedade `projectDir`:

```groovy
include ':app'
project(':app').projectDir = file('android/app')
```

---

## 2. Dependências Maven do Shizuku API
Em vez de depender de arquivos `.jar` locais extraídos temporariamente, a configuração oficial consome as dependências publicadas no MavenCentral:

```groovy
dependencies {
    implementation 'dev.rikka.shizuku:api:13.1.5'
    implementation 'dev.rikka.shizuku:provider:13.1.5'
}
```

---

## 3. Fluxo de Build Nativo C++ via CMake
O Android Gradle Plugin compilará nativamente a `libgimi_arm64.so` durante o comando `assembleDebug` utilizando a diretiva `externalNativeBuild`:

```groovy
android {
    externalNativeBuild {
        cmake {
            path file("../../CMakeLists.txt")
            version "3.22.1"
        }
    }
}
```

---

## Conclusion
A configuração integrada permite que a compilação completa ocorra via `gradle assembleDebug` gerando o APK final em `android/app/build/outputs/apk/debug/app-debug.apk`.
