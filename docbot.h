#pragma once

enum class DBLogLevel {
  Debug,
  Info,
  Warning,
  Error,
  Fatal
};

void dbLogPrintf(DBLogLevel level, const char* file, int line, const char* format, ...) 
  __attribute__ ((format (printf, 4, 5)));


void dbLogSetLevel(DBLogLevel level);

#define DBLOG(LEVEL, FORMAT, ...) do { dbLogPrintf(LEVEL, __FILE__, __LINE__, FORMAT, ## __VA_ARGS__); } while (0)
#define DBLOG_DEBUG(FORMAT, ...) DBLOG(DBLogLevel::Debug, FORMAT, ## __VA_ARGS__)
#define DBLOG_INFO(FORMAT, ...) DBLOG(DBLogLevel::Info, FORMAT, ## __VA_ARGS__)
#define DBLOG_WARN(FORMAT, ...) DBLOG(DBLogLevel::Warning, FORMAT, ## __VA_ARGS__)
#define DBLOG_ERROR(FORMAT, ...) DBLOG(DBLogLevel::Error, FORMAT, ## __VA_ARGS__)
#define DBLOG_FATAL(FORMAT, ...) DBLOG(DBLogLevel::Fatal, FORMAT, ## __VA_ARGS__)