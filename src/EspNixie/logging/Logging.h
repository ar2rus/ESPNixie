#pragma once

#include <stdarg.h>

class Logger;

enum LoggingLevel {
  LoggingLevelDebug,
  LoggingLevelInfo,
  LoggingLevelError,

  // Following levels are used when relaying logs received from WiFi module by host
  LoggingLevelESPDebug,
  LoggingLevelESPInfo,
  LoggingLevelESPError
};

class Logger {
  public:
    virtual void log(enum LoggingLevel level, const char* filename, int lineNumber, const char *fmt, va_list fmtargs) = 0;
};

class LoggingClass {
  private:
    Logger *_logger;

  public:

    LoggingClass() : _logger(0) { };

    void log(enum LoggingLevel level, const char* filename, int lineNumber, const char *fmt, ...);

    /**
     * Change the logger.
     */
    void setLogger(Logger *logger);
};

extern class LoggingClass Logging;


#define DEBUG(...) Logging.log(LoggingLevelDebug, __FILE__, __LINE__, __VA_ARGS__)
#define INFO(...) Logging.log(LoggingLevelInfo, __FILE__, __LINE__, __VA_ARGS__)
#define ERROR(...) Logging.log(LoggingLevelError, __FILE__, __LINE__, __VA_ARGS__)

