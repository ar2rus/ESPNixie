#include <Arduino.h>
#include <stdarg.h>

#include "Logging.h"

// Instantiate the shared instance that will be used.
LoggingClass Logging;

void LoggingClass::setLogger(Logger *logger) {
  _logger = logger;
}

void LoggingClass::log(enum LoggingLevel level, const char *fname, int lineno, const char *fmt, ... ) {
  if (!_logger) {
    return;
  }
  else {
    va_list fmtargs;
    va_start(fmtargs, fmt);
    _logger->log(level, fname, lineno, fmt, fmtargs);
    va_end(fmtargs);
  }
}
