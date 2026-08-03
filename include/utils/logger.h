#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Native Logging Utility
//
// Wraps <android/log.h> with printf-style macros and a lightweight
// structured logger. All output is tagged "gimi_arm64" and visible in
// Android Logcat: `adb logcat -s gimi_arm64`
//
// Usage:
//   LOGI("Layer loaded, version %s", GIMI_VERSION);
//   LOGE("Failed to hook %s: errno=%d", "eglGetProcAddress", errno);
// ─────────────────────────────────────────────────────────────────────────────

#include <android/log.h>
#include <cstdarg>

namespace gimi {

// ─── Version ──────────────────────────────────────────────────────────────────
constexpr const char* GIMI_VERSION  = "0.1.0";
constexpr const char* LOG_TAG       = "gimi_arm64";

// ─── Log Level Enum ───────────────────────────────────────────────────────────
enum class LogLevel : int {
    Debug   = ANDROID_LOG_DEBUG,
    Info    = ANDROID_LOG_INFO,
    Warn    = ANDROID_LOG_WARN,
    Error   = ANDROID_LOG_ERROR,
    Fatal   = ANDROID_LOG_FATAL,
};

// ─── Core Logging Function ────────────────────────────────────────────────────
void log_print(LogLevel level, const char* tag, const char* fmt, ...) noexcept;

} // namespace gimi

// ─── Convenience Macros ───────────────────────────────────────────────────────
// These forward to __android_log_print for zero-overhead release builds when
// NDEBUG is defined (LOGD becomes a no-op).

#ifdef NDEBUG
#   define LOGD(...)  ((void)0)
#else
#   define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, gimi::LOG_TAG, __VA_ARGS__)
#endif

#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,  gimi::LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,  gimi::LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, gimi::LOG_TAG, __VA_ARGS__)
#define LOGF(...)  __android_log_print(ANDROID_LOG_FATAL, gimi::LOG_TAG, __VA_ARGS__)
