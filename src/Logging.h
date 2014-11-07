#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

class Logger {
 public:
    Logger()
        : loglevel_(TRACE)
    { }

    enum LogLevel {
        TRACE,
        DEBUG,
        WARN,
        INFO,
        ERROR,
        FATAL
    };

    LogLevel setLevel(LogLevel level) {
        LogLevel prevLevel = loglevel_;
        loglevel_ = level;
        return prevLevel;
    }

    const char * to_string(LogLevel ll) {
        switch (ll) {
            case TRACE: return "TRACE"; break;
            case DEBUG: return "DEBUG"; break;
            case WARN: return "WARN"; break;
            case INFO: return "INFO"; break;
            case ERROR: return "ERROR"; break;
            case FATAL: return "FATAL"; break;
        }
    }

    const char * to_color(LogLevel ll) {
        switch (ll) {
            case TRACE: return ""; break;
            case DEBUG: return ""; break;
            case WARN: return ""; break;
            case INFO: return "\033[37m"; break;
            case ERROR: return "\033[31m"; break;
            case FATAL: return ""; break;
        }        
    }

    LogLevel logLevel() const {
        return loglevel_;
    }

    void log(LogLevel level, const std::string &msg) {
        std::time_t now = std::time(nullptr);
        std::clog << to_color(level)
                  << std::put_time(std::gmtime(&now), "%FT%T%z")
                  << " [" << to_string(level) << "] "
                  << msg 
                  << "\033[0m" << std::endl;
    }

 private:
    LogLevel loglevel_;
};

#define LOG(logger, level, msg)         \
    if (logger.logLevel() <= level) {   \
        std::stringstream ss;           \
        ss << msg;                      \
        logger.log(level, ss.str());    \
    }                                   \

#endif