#pragma once

#include <fstream>
#include <mutex>
#include <sstream>
#include <string>

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3,
};

class Logger {
public:
    static Logger& instance();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void setMinLevel(LogLevel level);
    void setConsoleEnabled(bool enabled);
    void setLogFile(const std::string& path);

    void write(LogLevel level, const char* file, int line, const std::string& message);

    class LogStream {
    public:
        LogStream(LogLevel level, const char* file, int line);
        ~LogStream();

        template <typename T>
        LogStream& operator<<(const T& value) {
            stream_ << value;
            return *this;
        }

    private:
        LogLevel level_;
        const char* file_;
        int line_;
        std::ostringstream stream_;
    };

private:
    Logger() = default;

    std::string format(LogLevel level, const char* file, int line, const std::string& message) const;
    static const char* levelName(LogLevel level);
    static const char* fileBaseName(const char* path);

    std::mutex mu_;
    LogLevel minLevel_ = LogLevel::Info;
    bool consoleEnabled_ = true;
    std::ofstream file_;
};

#define LOG_DEBUG Logger::LogStream(LogLevel::Debug, __FILE__, __LINE__)
#define LOG_INFO Logger::LogStream(LogLevel::Info, __FILE__, __LINE__)
#define LOG_WARN Logger::LogStream(LogLevel::Warn, __FILE__, __LINE__)
#define LOG_ERROR Logger::LogStream(LogLevel::Error, __FILE__, __LINE__)
