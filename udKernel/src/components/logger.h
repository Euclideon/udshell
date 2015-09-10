#pragma once
#ifndef UD_LOGGER_H
#define UD_LOGGER_H

#include "component.h"

namespace ud
{
PROTOTYPE_COMPONENT(Logger);
SHARED_CLASS(Stream);

/**
* --- LogCategories ---
*
* Error: Emit when the condition should never be encountered at runtime.
*
* Warning: Emit when program state looks questionable, but not proofably an error.
* Levels:
*   0 - Warning will be always be displayed. For cases that are very likely errors.
*   1 - Critical Warning. Application will probably not run correctly.
*   2 - Non-Critical Warning. Application will run, but may perform incorrectly.
*   3 - Low-confidence warning, likely correct program behaviour.
*   4 - High-frequency non-critical warnings. For warnings that appear in loops.
*
* Debug: Emit strictly for debug purposes, they are not of interest to users who are not debugging.
* Levels:
*   0 - Messages will be always be displayed.
*   1 - High-importance message.
*   2 - Normal-importance message.
*   3 - Typically unwanted message.
*   4 - Spammy high-frequency message.
*
* Info: Emit to the general output stream, may be viewed by end-users.
* Levels:
*   0 - Messages will be always be displayed.
*   1 - High-importance message.
*   2 - Normal-importance message.
*   3 - Typically unwanted message.
*   4 - Spammy high-frequency message.
*
* Script: Messages are associated with script execution; directed to script output areas.
*
* Trace: Used to follow the program flow. It is not unusual to place LogTrace() calls at the top of functions.
*/
UD_BITFIELD(LogCategories,
  Error,
  Warning,
  Debug,
  Info,
  Script,
  Trace
);

UD_BITFIELD(LogFormatSpecs,
  Timestamp,
  Category,
  Level,
  ComponentUID
);

UD_ENUM(LogDefaults,
  LogLevel = 2,
  StreamLevel = 2,
  Categories = (LogCategories::Error | LogCategories::Warning | LogCategories::Debug | LogCategories::Info | LogCategories::Script | LogCategories::Trace),
  Format = (LogFormatSpecs::Timestamp | LogFormatSpecs::Category | LogFormatSpecs::Level | LogFormatSpecs::ComponentUID)
);

class Logger : public Component
{
public:
  UD_COMPONENT(Logger);

  struct LogStream
  {
    LogStream(StreamRef spStream, LogCategories categories, int level, LogFormatSpecs format) :
      spStream(spStream), categories(categories), level(level), format(format) {}

    StreamRef spStream;
    LogCategories categories;
    int level;
    LogFormatSpecs format;
  };

  int Log(int level, udString text, LogCategories category = LogCategories::Debug, udString componentUID = nullptr);

  bool GetEnabled() const { return bEnabled; }
  void SetEnabled(bool bEnable) { this->bEnabled = bEnable; }

  void AddStream(StreamRef spStream, LogCategories categories = LogDefaults::Categories, int level = LogDefaults::StreamLevel, LogFormatSpecs format = LogDefaults::Format);
  int RemoveStream(StreamRef spStream);

  int GetLevel(StreamRef spStream) const;
  int SetLevel(StreamRef spStream, int level);

  int AddCategory(StreamRef spStream, LogCategories category);
  int RemoveCategory(StreamRef spStream, LogCategories category);
  LogCategories GetCategories(StreamRef spStream) const;
  int SetCategories(StreamRef spStream, LogCategories categories);

protected:
  Logger(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);

  LogStream *FindLogStream(StreamRef spStream) const;

  udFixedSlice<LogStream, 1> streamList;
  bool bEnabled;
};

} //namespace ud
#endif // UDLOGGER_H
