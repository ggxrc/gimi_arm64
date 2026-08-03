#!/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
# Helper script to generate android/debug.keystore if missing
# ─────────────────────────────────────────────────────────────────────────────

KEYSTORE_PATH="$(dirname "$0")/debug.keystore"

if [ -f "$KEYSTORE_PATH" ]; then
    echo "🔑 Keystore already exists: $KEYSTORE_PATH"
    exit 0
fi

echo "🔑 Generating Android debug keystore..."

if command -v keytool >/dev/null 2>&1; then
    keytool -genkeypair -v \
        -keystore "$KEYSTORE_PATH" \
        -storepass android \
        -alias androiddebugkey \
        -keypass android \
        -keyalg RSA \
        -keysize 2048 \
        -validity 10000 \
        -dname "CN=Android Debug,O=Android,C=US"
elif command -v openssl >/dev/null 2>&1; then
    TEMP_KEY=$(mktemp)
    TEMP_CERT=$(mktemp)
    openssl req -x509 -newkey rsa:2048 -keyout "$TEMP_KEY" -out "$TEMP_CERT" -days 10000 -nodes -subj "/CN=Android Debug/O=Android/C=US" 2>/dev/null
    openssl pkcs12 -export -in "$TEMP_CERT" -inkey "$TEMP_KEY" -out "$KEYSTORE_PATH" -name androiddebugkey -passout pass:android 2>/dev/null
    rm -f "$TEMP_KEY" "$TEMP_CERT"
else
    echo "❌ Neither keytool nor openssl found! Unable to generate $KEYSTORE_PATH"
    exit 1
fi

if [ -f "$KEYSTORE_PATH" ]; then
    echo "✅ Keystore generated successfully: $KEYSTORE_PATH"
else
    echo "❌ Failed to generate keystore."
    exit 1
fi
