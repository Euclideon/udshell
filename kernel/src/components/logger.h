#pragma once
#ifndef EP_LOGGER_H
#define EP_LOGGER_H

#include "component.h"

namespace ep
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
#define NUM_LOG_CATEGORIES  6

UD_BITFIELD(LogFormatSpecs,
  Timestamp,
  Category,
  Level,
  ComponentUID
);

UD_ENUM(LogDefaults,
  LogLevel = 2,
  StreamLevel = 5,
  Categories = (LogCategories::Error | LogCategories::Warning | LogCategories::Debug | LogCategories::Info | LogCategories::Script | LogCategories::Trace),
  Format = (LogFormatSpecs::Timestamp | LogFormatSpecs::Category | LogFormatSpecs::Level | LogFormatSpecs::ComponentUID)
);

class Logger : public Component
{
public:
  EP_COMPONENT(Logger);

  struct LogStream
  {
    LogStream(StreamRef spStream, LogCategories categories, int level, LogFormatSpecs format) :
      spStream(spStream), categories(categories), level(level), format(format) {}

    StreamRef spStream;
    LogCategories categories;
    int level;
    LogFormatSpecs format;
  };

  void Log(int level, epString text, LogCategories category = LogCategories::Debug, epString componentUID = nullptr);

  bool GetEnabled() const { return bEnabled; }
  void SetEnabled(bool bEnable) { this->bEnabled = bEnable; }

  void AddStream(StreamRef spStream, LogCategories categories = LogDefaults::Categories, int level = LogDefaults::StreamLevel, LogFormatSpecs format = LogDefaults::Format);
  int RemoveStream(StreamRef spStream);

  int GetLevel(StreamRef spStream) const;
  int SetLevel(StreamRef spStream, int level);
  LogCategories GetCategories(StreamRef spStream) const;
  int SetCategories(StreamRef spStream, LogCategories categories);

  int GetFilterLevel(LogCategories category) const;
  void SetFilterLevel(LogCategories categories, int level);
  epSlice<epSharedString> GetFilterComponents() const;
  void SetFilterComponents(epSlice<const epString> comps);
  void RemoveFilters();
protected:
  Logger(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);

  LogStream *FindLogStream(StreamRef spStream) const;

  epArray<LogStream, 1> streamList;
  bool bEnabled = true, bLogging = false;

  int levelsFilter[NUM_LOG_CATEGORIES];
  epArray<epSharedString> componentsFilter;
};

} //namespace ep
#endif // EP_LOGGER_H
