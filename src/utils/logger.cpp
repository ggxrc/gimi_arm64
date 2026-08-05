// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Native Logging Utility Implementation
//
// Provides:
//   1. Formatted Android logcat output (existing)
//   2. File-based render logging to /sdcard/GIMI/gimi_render.log (new)
// ─────────────────────────────────────────────────────────────────────────────

#include "utils/logger.h"
#include <android/log.h>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <sys/stat.h>

namespace gimi {

// ─── Core logcat logger ──────────────────────────────────────────────────────
void log_print(LogLevel level, const char* tag, const char* fmt, ...) noexcept {
    // Compose the formatted message into a stack buffer (avoids heap alloc
    // inside render-critical paths while still supporting long messages).
    char buf[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    __android_log_print(static_cast<int>(level), tag, "%s", buf);
}

// ─── FileLogger singleton ─────────────────────────────────────────────────────
FileLogger& FileLogger::instance() noexcept {
    static FileLogger s_instance;
    return s_instance;
}

FileLogger::~FileLogger() {
    close();
}

void FileLogger::init(const std::string& path) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_open.load(std::memory_order_relaxed)) return;

    // Ensure parent directory exists
    std::string dir = path.substr(0, path.rfind('/'));
    if (!dir.empty()) {
        mkdir(dir.c_str(), 0755);
    }

    m_file = fopen(path.c_str(), "a");
    if (m_file) {
        m_open.store(true, std::memory_order_release);

        // Write session header
        time_t now = time(nullptr);
        char timebuf[64];
        struct tm tm_info;
        localtime_r(&now, &tm_info);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm_info);

        fprintf(m_file,
            "\n"
            "═══════════════════════════════════════════════════════════════\n"
            "  GIMI ARM64 Render Log — Session %s\n"
            "  Version: %s\n"
            "═══════════════════════════════════════════════════════════════\n",
            timebuf, GIMI_VERSION);
        fflush(m_file);

        LOGI("FileLogger: opened %s", path.c_str());
    } else {
        LOGE("FileLogger: failed to open %s", path.c_str());
    }
}

void FileLogger::write(const char* level, const char* fmt, ...) noexcept {
    if (!m_open.load(std::memory_order_acquire)) return;

    char msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    // Get timestamp
    time_t now = time(nullptr);
    char timebuf[32];
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", &tm_info);

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file) {
        fprintf(m_file, "[%s] [%s] %s\n", timebuf, level, msg);
        // Flush every write to avoid losing data on crash
        fflush(m_file);
    }
}

void FileLogger::flush() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file) {
        fflush(m_file);
    }
}

void FileLogger::close() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file) {
        fflush(m_file);
        fclose(m_file);
        m_file = nullptr;
        m_open.store(false, std::memory_order_release);
    }
}

// ─── Render log (logcat + file) ──────────────────────────────────────────────
void log_render(const char* fmt, ...) noexcept {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // Output to logcat
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "[RENDER] %s", buf);

    // Output to file
    FileLogger::instance().write("RENDER", "%s", buf);
}

} // namespace gimi
