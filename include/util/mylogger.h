#pragma once
#include <iostream>
#include <sstream>
#include <mutex>
#include <sys/syscall.h>
#include <unistd.h>
#include <thread>

namespace myreactor {

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    Logger(const char* file, int line, LogLevel level);
    ~Logger();

    std::ostringstream& stream() { return ss_; }

private:
    static const char* levelToString(LogLevel level) {
        switch (level) {
            case DEBUG:
                return "DEBUG";
            case INFO:
                return "INFO";
            case WARN:
                return "WARN";
            case ERROR:
                return "ERROR";
            default:
                return "UNKNOWN";
        }
    }

    std::ostringstream ss_;
    LogLevel level_;
};

#define LOG_DEBUG Logger(__FILE__, __LINE__, DEBUG).stream()
#define LOG_INFO Logger(__FILE__, __LINE__, INFO).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, ERROR).stream()

}