#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Native Logging Utility
//
// Wraps <android/log.h> with printf-style macros and a lightweight
// structured logger. All output is tagged "gimi_arm64" and visible in
// Android Logcat: `adb logcat -s gimi_arm64`
//
// File logging: render-critical events are also written to
// /sdcard/GIMI/gimi_render.log via the FileLogger singleton.
//
// Usage:
//   LOGI("Layer loaded, version %s", GIMI_VERSION);
//   LOGE("Failed to hook %s: errno=%d", "eglGetProcAddress", errno);
//   LOGR("DrawCall hash=0x%08X matched override", hash);  // → logcat + file
// ─────────────────────────────────────────────────────────────────────────────

#include <android/log.h>
#include <cstdarg>
#include <string>
#include <mutex>
#include <atomic>

namespace gimi {

// ─── Version ──────────────────────────────────────────────────────────────────
constexpr const char* GIMI_VERSION  = "1.0.0";
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

// ─── File Logger ──────────────────────────────────────────────────────────────
// Thread-safe file logger that writes render events to /sdcard/GIMI/gimi_render.log.
// Designed for low overhead: formats on caller thread, writes under a mutex.
class FileLogger {
public:
    static FileLogger& instance() noexcept;

    // Initialize (open) the log file. Call once at layer init.
    void init(const std::string& path = "/sdcard/GIMI/gimi_render.log") noexcept;

    // Write a formatted line to the log file (thread-safe).
    void write(const char* level, const char* fmt, ...) noexcept;

    // Flush the file to disk.
    void flush() noexcept;

    // Close the log file.
    void close() noexcept;

    bool is_open() const noexcept { return m_open.load(std::memory_order_relaxed); }

private:
    FileLogger() = default;
    ~FileLogger();

    std::mutex  m_mutex;
    FILE*       m_file  = nullptr;
    std::atomic<bool> m_open{false};
};

// ─── Render Log Function ──────────────────────────────────────────────────────
// Writes to both logcat (INFO) and the render log file.
void log_render(const char* fmt, ...) noexcept;

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

// Render log: logcat + /sdcard/GIMI/gimi_render.log
#define LOGR(...)  gimi::log_render(__VA_ARGS__)
