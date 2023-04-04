
#include "docbot.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

static DBLogLevel sLevel = DBLogLevel::Debug;


void dbLogSetLevel(DBLogLevel level)
{
    sLevel = level;
}

void dbLogPrintf(DBLogLevel level, const char* file, int line, const char* format, ...) 
{
  if (level < sLevel)
    return;
  
  // convert level to string
  const char* level_str = nullptr;
  switch (level) {
    case DBLogLevel::Debug:
      level_str = "DEBUG";
      break;
    case DBLogLevel::Info:
      level_str = "INFO";
      break;
    case DBLogLevel::Warning:
      level_str = "WARN";
      break;
    case DBLogLevel::Error:
      level_str = "ERROR";
      break;
    case DBLogLevel::Fatal:
      level_str = "FATAL";
      break;
  }

  // trim the file name up until the last '/'
  const char* file_name = file;
  const char* last_slash = strrchr(file, '/');
  if (last_slash)
    file_name = last_slash + 1;

  char buff[1024];

  va_list args;
  va_start(args, format);
  vsnprintf(buff, sizeof(buff), format, args);
  va_end(args);

  // print the current date and time
  time_t now = time(0);
  struct tm tstruct;
  char time_buff[80];
  tstruct = *localtime(&now);
  strftime(time_buff, sizeof(time_buff), "%Y-%m-%d.%X", &tstruct);

  // print the log message
  fprintf(stderr, "%s %s %s:%d %s\n", time_buff, level_str, file_name, line, buff);  
}