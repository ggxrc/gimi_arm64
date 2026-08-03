// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Native Logging Utility Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "utils/logger.h"
#include <android/log.h>
#include <cstdarg>
#include <cstdio>

namespace gimi {

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

} // namespace gimi
