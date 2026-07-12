#include "mylogger.h"

namespace myreactor {

LogLevel g_logLevel = DEBUG;

Logger::Logger(const char* file, int line, LogLevel level): level_(level) {
    ss_ << "[" << levelToString(level_) << "]" 
        << "[" << "tid:" << std::this_thread::get_id() << "]"
        << " " << file << ":"
        << line << " ";
}
        
Logger::~Logger() { 
    if (level_ < g_logLevel) 
        return;
    // ss_ << "\n";
    static std::mutex mutex_;   // 所有日志对象共享一把锁
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << ss_.str() << std::endl;
}


}

