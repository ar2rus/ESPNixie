#include "LoggerStream.h"

static int strrpos(const char *string, char c) {
  int index = -1;
  int i = 0;
  for (i = 0; string[i] != 0; i++) {
    if (string[i] == c) {
      index = i;
    }
  }
  return index;
}

LoggerStream::LoggerStream(Stream &s) : _stream(s) {
}

void LoggerStream::log(enum LoggingLevel level, const char *fname, int lineno, const char *fmt, va_list fmtargs) {
  static const char *logLevelPrefixes[] = { "D", "I", "E", "WD", "WI", "WE" };

  int lastSlash = strrpos(fname, '/');
  if (lastSlash > 0) {
    fname = fname + lastSlash + 1;
  }
  _stream.print(logLevelPrefixes[level]);
  _stream.print(" ");
  _stream.print(fname);
  _stream.print(":");
  _stream.print(lineno);
  _stream.print(" ");

  char tmp[128];
  vsnprintf(tmp, sizeof(tmp), fmt, fmtargs);

  _stream.println(tmp);
};
