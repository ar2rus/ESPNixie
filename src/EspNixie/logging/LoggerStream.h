#pragma once

#include <Stream.h>
#include "Logging.h"

class LoggerStream: public Logger {
  private:
    Stream &_stream;

  public:
    LoggerStream(Stream &s);

    void log(enum LoggingLevel level, const char *fname, int lineno, const char *fmt, va_list fmtargs);
};
