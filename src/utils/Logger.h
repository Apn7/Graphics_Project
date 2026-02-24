// =============================================================================
// Logger.h — Simple Console Logger with Color-Coded Output
// =============================================================================
// A header-only logging utility that prints timestamped, color-coded messages
// to the console. Supports three severity levels: INFO, WARN, and ERROR.
//
// Usage:
//   LOG_INFO("Window created successfully");
//   LOG_WARN("Texture not found, using default");
//   LOG_ERROR("Shader compilation failed");
//
// Output format:
//   [HH:MM:SS] [INFO]  Window created successfully     (green)
//   [HH:MM:SS] [WARN]  Texture not found, using default (yellow)
//   [HH:MM:SS] [ERROR] Shader compilation failed        (red)
// =============================================================================

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>     // std::cout, std::cerr
#include <string>       // std::string
#include <chrono>       // std::chrono for timestamps
#include <ctime>        // std::localtime, std::strftime
#include <iomanip>      // std::put_time (fallback)

// ---- ANSI Color Codes ----
// These escape sequences colorize terminal output on supported terminals.
// On Windows 10+, ANSI codes are supported in the default console.
namespace LogColor {
    constexpr const char* RESET  = "\033[0m";       // Reset to default
    constexpr const char* GREEN  = "\033[32m";      // INFO messages
    constexpr const char* YELLOW = "\033[33m";      // WARN messages
    constexpr const char* RED    = "\033[31m";       // ERROR messages
    constexpr const char* GRAY   = "\033[90m";      // Timestamp
}

// ---- Log Severity Levels ----
enum class LogLevel {
    INFO,
    WARN,
    ERR     // Avoiding 'ERROR' — conflicts with Windows macro
};

// =============================================================================
// GetTimestamp — Returns the current time as a formatted string [HH:MM:SS]
// =============================================================================
inline std::string GetTimestamp() {
    // Get current system time
    auto now = std::chrono::system_clock::now();
    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);

    // Convert to local time struct
    std::tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &timeNow);  // Thread-safe version on Windows
#else
    localtime_r(&timeNow, &localTime);  // Thread-safe version on POSIX
#endif

    // Format as [HH:MM:SS]
    char buffer[12];
    std::strftime(buffer, sizeof(buffer), "[%H:%M:%S]", &localTime);
    return std::string(buffer);
}

// =============================================================================
// LogMessage — Core logging function. Prints a color-coded, timestamped message.
// =============================================================================
inline void LogMessage(LogLevel level, const std::string& message) {
    // Select color and label based on severity level
    const char* color = LogColor::RESET;
    const char* label = "[INFO] ";

    switch (level) {
        case LogLevel::INFO:
            color = LogColor::GREEN;
            label = "[INFO] ";
            break;
        case LogLevel::WARN:
            color = LogColor::YELLOW;
            label = "[WARN] ";
            break;
        case LogLevel::ERR:
            color = LogColor::RED;
            label = "[ERROR]";
            break;
    }

    // Print: timestamp (gray) + label (colored) + message (colored) + reset
    std::cout << LogColor::GRAY << GetTimestamp() << " "
              << color << label << " " << message
              << LogColor::RESET << std::endl;
}

// =============================================================================
// Convenience Macros — Use these throughout the project
// =============================================================================
// These macros provide a clean, short syntax for logging at each level.

#define LOG_INFO(msg)  LogMessage(LogLevel::INFO, msg)
#define LOG_WARN(msg)  LogMessage(LogLevel::WARN, msg)
#define LOG_ERROR(msg) LogMessage(LogLevel::ERR,  msg)

#endif // LOGGER_H
