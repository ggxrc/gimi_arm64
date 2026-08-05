#!/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
# gimi_arm64 — Lightweight Termux Build & Packaging Script
# Compiles C++20 libgimi_arm64.so, processes Android resources, compiles
# Kotlin & Java sources (kotlinc + D8), and signs GIMI-Launcher.apk.
# ─────────────────────────────────────────────────────────────────────────────

set -e

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}=====================================================${NC}"
echo -e "${CYAN}🚀 gimi_arm64 — Termux Build & APK Packaging Pipeline${NC}"
echo -e "${CYAN}=====================================================${NC}"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

export PREFIX="${PREFIX:-/data/data/com.termux/files/usr}"

# ─── 1. Dependency Detection ─────────────────────────────────────────────────
echo -e "\n${YELLOW}🔍 Checking Termux build dependencies...${NC}"

MISSING_DEPS=()

check_cmd() {
    local cmd=$1
    local pkg=$2
    if command -v "$cmd" >/dev/null 2>&1; then
        echo -e "  [✔] $cmd: $(command -v "$cmd")"
        return 0
    else
        echo -e "  [❌] $cmd missing (package: $pkg)"
        MISSING_DEPS+=("$pkg")
        return 1
    fi
}

check_cmd cmake "cmake"
check_cmd clang "clang"
check_cmd javac "openjdk-21"
check_cmd kotlinc "kotlin"

# Zip tool check with python3 / jar fallback
ZIP_CMD=""
if command -v zip >/dev/null 2>&1; then
    ZIP_CMD="zip"
    echo -e "  [✔] zip: $(command -v zip)"
elif command -v python3 >/dev/null 2>&1; then
    ZIP_CMD="python3"
    echo -e "  [✔] zip: python3 zipfile module"
elif command -v jar >/dev/null 2>&1; then
    ZIP_CMD="jar"
    echo -e "  [✔] zip: jar utility"
else
    echo -e "  [❌] zip missing (package: zip or python)"
    MISSING_DEPS+=("zip")
fi

# AAPT/AAPT2 check
AAPT_CMD=""
if command -v aapt2 >/dev/null 2>&1; then
    AAPT_CMD="aapt2"
    echo -e "  [✔] aapt2: $(command -v aapt2)"
elif command -v aapt >/dev/null 2>&1; then
    AAPT_CMD="aapt"
    echo -e "  [✔] aapt: $(command -v aapt)"
else
    echo -e "  [❌] aapt/aapt2 missing (package: aapt)"
    MISSING_DEPS+=("aapt")
fi

# D8/DX check
D8_CMD=""
if command -v d8 >/dev/null 2>&1; then
    D8_CMD="d8"
    echo -e "  [✔] d8: $(command -v d8)"
elif command -v dx >/dev/null 2>&1; then
    D8_CMD="dx"
    echo -e "  [✔] dx: $(command -v dx)"
elif command -v ecj >/dev/null 2>&1; then
    D8_CMD="ecj"
    echo -e "  [✔] ecj: $(command -v ecj)"
else
    echo -e "  [⚠️] d8/dx missing (package: ecj or android-tools)"
fi

check_cmd zipalign "zipalign"
check_cmd apksigner "apksigner"

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    echo -e "\n${RED}⚠️ Missing required packages. You can install them in Termux via:${NC}"
    echo -e "${YELLOW}pkg install openjdk-21 cmake clang aapt zipalign apksigner ecj zip kotlin${NC}\n"
fi

# ─── 2. Ensure Android Debug Keystore ─────────────────────────────────────────
echo -e "\n${YELLOW}🔑 Ensuring android/debug.keystore...${NC}"
bash android/generate_keystore.sh || true

# ─── 3. Locate or Download minimal android.jar ────────────────────────────────
echo -e "\n${YELLOW}📦 Locating android.jar...${NC}"

ANDROID_JAR=""
JAR_CANDIDATES=(
    "$ROOT_DIR/.android-sdk/platforms/android-34/android.jar"
    "$ROOT_DIR/build/android.jar"
    "$ROOT_DIR/android/android.jar"
    "$ANDROID_HOME/platforms/android-34/android.jar"
    "$ANDROID_HOME/platforms/android-33/android.jar"
    "$PREFIX/share/android-sdk/platforms/android-34/android.jar"
)

for jar in "${JAR_CANDIDATES[@]}"; do
    if [ -n "$jar" ] && [ -f "$jar" ]; then
        SIZE=$(wc -c < "$jar" 2>/dev/null || echo 0)
        if [ "$SIZE" -gt 1000000 ]; then
            ANDROID_JAR="$jar"
            break
        fi
    fi
done

if [ -z "$ANDROID_JAR" ]; then
    echo -e "  [⚠️] Valid android.jar not found in local paths. Downloading minimal android.jar..."
    mkdir -p build
    ANDROID_JAR_URL="https://raw.githubusercontent.com/Sable/android-platforms/master/android-30/android.jar"
    
    if command -v curl >/dev/null 2>&1; then
        curl -sSL -o build/android.jar "$ANDROID_JAR_URL" || true
    elif command -v wget >/dev/null 2>&1; then
        wget -q -O build/android.jar "$ANDROID_JAR_URL" || true
    fi

    if [ -f "build/android.jar" ]; then
        ANDROID_JAR="$ROOT_DIR/build/android.jar"
        echo -e "  [✔] Downloaded android.jar to build/android.jar"
    else
        echo -e "  [❌] Failed to download android.jar. APK packaging may fail."
    fi
else
    echo -e "  [✔] Found android.jar at: $ANDROID_JAR"
fi

# ─── 4. Compile Native C++20 Library (libgimi_arm64.so) ──────────────────────
echo -e "\n${YELLOW}🛠️ Compiling C++20 native targets via CMake...${NC}"
mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_SKIP_RPATH=ON -DCMAKE_SKIP_BUILD_RPATH=ON
make -j$(nproc)

cd "$ROOT_DIR"

if [ -f "build/libgimi_arm64.so" ]; then
    echo -e "  [✔] Built build/libgimi_arm64.so"
else
    echo -e "  [❌] Native library build failed!"
    exit 1
fi

# ─── 5. Compile Android Resources, Kotlin & Java Launcher Code ───────────────
echo -e "\n${YELLOW}📱 Compiling Android resources and Kotlin/Java sources...${NC}"

mkdir -p build/gen build/obj build/dex build/apk_contents

# Package resources
RESOURCE_AP_="build/resources.ap_"

if [ "$AAPT_CMD" = "aapt2" ]; then
    aapt2 compile --dir app/src/main/res -o build/compiled_res.zip 2>/dev/null || true
    if [ -f "$ANDROID_JAR" ]; then
        aapt2 link -o "$RESOURCE_AP_" -I "$ANDROID_JAR" \
            --min-sdk-version 29 --target-sdk-version 36 \
            --manifest app/src/main/AndroidManifest.xml \
            build/compiled_res.zip --java build/gen --auto-add-overlay 2>/dev/null || true
    fi
elif [ "$AAPT_CMD" = "aapt" ]; then
    if [ -f "$ANDROID_JAR" ]; then
        aapt package -f -m -J build/gen -M app/src/main/AndroidManifest.xml \
            -S app/src/main/res -I "$ANDROID_JAR" -F "$RESOURCE_AP_" 2>/dev/null || true
    fi
fi

# Locate Kotlin and Java sources
JAVA_FILES=$(find app/src/main/java build/gen -name "*.java" 2>/dev/null)

if command -v javac >/dev/null 2>&1 && [ -f "$ANDROID_JAR" ] && [ -n "$JAVA_FILES" ]; then
    echo -e "  [✔] Compiling Java source files with javac (Java 8 bytecode)..."
    javac -encoding UTF-8 -source 1.8 -target 1.8 -cp "$ANDROID_JAR" -d build/obj $JAVA_FILES
fi

# Convert compiled class files to Dalvik bytecode (classes.dex)
CLASS_FILES=$(find build/obj -name "*.class" 2>/dev/null)
if [ -n "$CLASS_FILES" ]; then
    mkdir -p build/dex
    if [ "$D8_CMD" = "d8" ]; then
        echo -e "  [✔] Converting bytecode to classes.dex using d8..."
        d8 --lib "$ANDROID_JAR" --output build/dex $CLASS_FILES
    elif [ "$D8_CMD" = "dx" ]; then
        echo -e "  [✔] Converting bytecode to classes.dex using dx..."
        dx --dex --output=build/dex/classes.dex build/obj
    fi
fi

if [ ! -f "build/dex/classes.dex" ] || [ ! -s "build/dex/classes.dex" ]; then
    echo -e "  [❌] ERROR: classes.dex is missing or empty! Aborting build."
    exit 1
fi

# ─── 6. Assemble, Align & Sign GIMI-Launcher.apk ─────────────────────────────
echo -e "\n${YELLOW}📦 Assembling, aligning, and signing GIMI-Launcher.apk...${NC}"

# Extract base resources if available, otherwise prepare fresh structure
rm -rf build/apk_contents/*
mkdir -p build/apk_contents/lib/arm64-v8a

if [ -f "$RESOURCE_AP_" ]; then
    unzip -q -o "$RESOURCE_AP_" -d build/apk_contents/ 2>/dev/null || true
fi

# Copy dex and native library into APK contents
if [ ! -f "build/apk_contents/AndroidManifest.xml" ]; then
    cp app/src/main/AndroidManifest.xml build/apk_contents/ 2>/dev/null || true
fi
cp build/dex/classes.dex build/apk_contents/ 2>/dev/null || true
cp build/libgimi_arm64.so build/apk_contents/lib/arm64-v8a/
if [ -f "/data/data/com.termux/files/usr/lib/libc++_shared.so" ]; then
    cp /data/data/com.termux/files/usr/lib/libc++_shared.so build/apk_contents/lib/arm64-v8a/ 2>/dev/null || true
fi

cd build/apk_contents
if command -v zip >/dev/null 2>&1; then
    zip -q -r -0 ../GIMI-Launcher-unaligned.apk .
elif command -v python3 >/dev/null 2>&1; then
    python3 -m zipfile -c ../GIMI-Launcher-unaligned.apk .
elif command -v jar >/dev/null 2>&1; then
    jar cf0 ../GIMI-Launcher-unaligned.apk *
fi
cd "$ROOT_DIR"

UNALIGNED_APK="build/GIMI-Launcher-unaligned.apk"
ALIGNED_APK="build/GIMI-Launcher-aligned.apk"
FINAL_APK="GIMI-Launcher.apk"

# Zipalign
if command -v zipalign >/dev/null 2>&1 && [ -f "$UNALIGNED_APK" ]; then
    zipalign -f 4 "$UNALIGNED_APK" "$ALIGNED_APK"
else
    cp "$UNALIGNED_APK" "$ALIGNED_APK" 2>/dev/null || true
fi

# Apksigner
KEYSTORE="android/debug.keystore"
if command -v apksigner >/dev/null 2>&1 && [ -f "$KEYSTORE" ] && [ -f "$ALIGNED_APK" ]; then
    echo -e "  [✔] Signing APK with $KEYSTORE using apksigner..."
    apksigner sign --ks "$KEYSTORE" --ks-pass pass:android --ks-key-alias androiddebugkey --key-pass pass:android --v1-signing-enabled true --v2-signing-enabled true --v3-signing-enabled true --min-sdk-version 26 --out "$FINAL_APK" "$ALIGNED_APK" 2>/dev/null || cp "$ALIGNED_APK" "$FINAL_APK"
else
    echo -e "  [⚠️] apksigner or keystore missing. Outputting unverified APK as $FINAL_APK."
    cp "$ALIGNED_APK" "$FINAL_APK" 2>/dev/null || true
fi

# Also output to standard AGP debug path app/build/outputs/apk/debug/app-debug.apk
mkdir -p app/build/outputs/apk/debug
cp "$FINAL_APK" app/build/outputs/apk/debug/app-debug.apk 2>/dev/null || true

# ─── 7. Summary & Verification ───────────────────────────────────────────────
echo -e "\n${GREEN}=====================================================${NC}"
echo -e "${GREEN}🎉 Build Process Completed!${NC}"
echo -e "${GREEN}=====================================================${NC}"

if [ -f "build/libgimi_arm64.so" ]; then
    echo -e "  - Native Library: ${CYAN}build/libgimi_arm64.so${NC} ($(du -h build/libgimi_arm64.so | cut -f1))"
fi

if [ -f "$FINAL_APK" ]; then
    echo -e "  - Launcher APK:   ${CYAN}$FINAL_APK${NC} ($(du -h "$FINAL_APK" | cut -f1))"
    echo -e "  - AGP Debug APK:  ${CYAN}app/build/outputs/apk/debug/app-debug.apk${NC}"
    if command -v apksigner >/dev/null 2>&1 && [ -f "$KEYSTORE" ]; then
        echo -e "  - Signature Status:"
        apksigner verify --min-sdk-version 26 "$FINAL_APK" && echo -e "    ${GREEN}✔ APK Signature Verified Cleanly!${NC}" || echo -e "    ${YELLOW}⚠️ Signature Verification Warning${NC}"
    fi
fi

echo -e "\n${GREEN}Done.${NC}"
