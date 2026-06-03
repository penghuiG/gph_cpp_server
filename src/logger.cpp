#include "logger.h"
#include <cstring>
#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setMinLevel(LogLevel level) {
    std::lock_guard<std::mutex> lk(mu_);
    minLevel_ = level;
}

void Logger::setConsoleEnabled(bool enabled) {
    std::lock_guard<std::mutex> lk(mu_);
    consoleEnabled_ = enabled;
}

void Logger::setLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lk(mu_);
    if (file_.is_open()) {
        file_.close();
    }
    if (path.empty()) {
        return;
    }

    const auto pos = path.find_last_of('/');
    if (pos != std::string::npos) {
        const std::string dir = path.substr(0, pos);
        mkdir(dir.c_str(), 0755);
    }

    file_.open(path, std::ios::app);
    if (!file_.is_open()) {
        std::cerr << "failed to open log file: " << path << std::endl;
    }
}

void Logger::write(LogLevel level, const char* file, int line, const std::string& message) {
    std::lock_guard<std::mutex> lk(mu_);
    if (level < minLevel_) {
        return;
    }

    const std::string formatted = format(level, file, line, message);
    if (consoleEnabled_) {
        std::ostream& out = (level >= LogLevel::Warn) ? std::cerr : std::cout;
        out << formatted << std::endl;
    }
    if (file_.is_open()) {
        file_ << formatted << std::endl;
        file_.flush();
    }
}

Logger::LogStream::LogStream(LogLevel level, const char* file, int line)
    : level_(level), file_(file), line_(line) {}

Logger::LogStream::~LogStream() {
    Logger::instance().write(level_, file_, line_, stream_.str());
}

std::string Logger::format(LogLevel level, const char* file, int line, const std::string& message) const {
    const auto now = std::chrono::system_clock::now();
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    const std::time_t sec = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
    localtime_r(&sec, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.'
        << std::setfill('0') << std::setw(3) << ms.count()
        << " [" << levelName(level) << "]"
        << " [" << fileBaseName(file) << ':' << line << "] "
        << message;
    return oss.str();
}

const char* Logger::levelName(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
    }
    return "UNKNOWN";
}

const char* Logger::fileBaseName(const char* path) {
    if (path == nullptr) {
        return "unknown";
    }
    const char* slash = std::strrchr(path, '/');
    if (slash != nullptr) {
        return slash + 1;
    }
    slash = std::strrchr(path, '\\');
    if (slash != nullptr) {
        return slash + 1;
    }
    return path;
}
