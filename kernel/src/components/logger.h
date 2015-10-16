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
EP_BITFIELD(LogCategories,
  Error,
  Warning,
  Debug,
  Info,
  Script,
  Trace
);
#define NUM_LOG_CATEGORIES  6

EP_BITFIELD(LogFormatSpecs,
  Timestamp,
  Category,
  Level,
  ComponentUID
);

EP_ENUM(LogDefaults,
  LogLevel = 2,
  StreamLevel = 5,
  Categories = (LogCategories::Error | LogCategories::Warning | LogCategories::Debug | LogCategories::Info | LogCategories::Script | LogCategories::Trace),
  Format = (LogFormatSpecs::Timestamp | LogFormatSpecs::Category | LogFormatSpecs::Level | LogFormatSpecs::ComponentUID)
);

struct LogLine
{
  LogLine(int level, SharedString text, LogCategories category, SharedString componentID);

  SharedString ToString(LogFormatSpecs format = LogDefaults::Format) const;

  int level;
  SharedString text;
  LogCategories category;
  SharedString componentUID;
  time_t timestamp;
  double ordering;
};

class LogFilter
{
public:
  // Log level filtering for streams
  int GetLevel(LogCategories category) const;
  void SetLevel(LogCategories categories, int level);
  Slice<SharedString> GetComponents() const;
  void SetComponents(Slice<const String> comps);
  void ResetFilter();
  bool FilterLogLine(LogLine &line) const;

  LogFilter()
  {
    ResetFilter();
  }
protected:
  int levelsFilter[NUM_LOG_CATEGORIES];
  Array<SharedString> componentsFilter;
};

struct LogStream
{
  LogStream(StreamRef spStream, LogFormatSpecs format, LogFilter filter) :
    spStream(spStream), filter(filter), format(format) {}

  LogStream &operator=(const LogStream &other)
  {
    // Array requires assignment for certain functionality - implement the assignment path if those features become needed
    EPASSERT(false, "LogStream assignment is unsupported");
    return *this;
  }

  StreamRef spStream;
  LogFilter filter;
  LogFormatSpecs format;
};

class Logger : public Component
{
public:
  EP_COMPONENT(Logger);

  void Log(int level, String text, LogCategories category = LogCategories::Debug, String componentUID = nullptr);

  bool GetEnabled() const { return bEnabled; }
  void SetEnabled(bool bEnable) { this->bEnabled = bEnable; }

  const Slice<LogLine> GetLog() const { return internalLog; }

  void AddStream(StreamRef spStream, LogFormatSpecs format = LogDefaults::Format);
  int RemoveStream(StreamRef spStream);

  // Stream helper functions
  LogStream *GetLogStream(StreamRef spStream);
  // Stream level filtering
  int GetStreamLevel(StreamRef spStream, LogCategories category) const;
  int SetStreamLevel(StreamRef spStream, LogCategories categories, int level);
  Slice<SharedString> GetStreamComponents(StreamRef spStream) const;
  int SetStreamComponents(StreamRef spStream, Slice<const String> comps);
  int ResetStreamFilter(StreamRef spStream);

  // Logger level filtering
  LogFilter &GetFilter() { return filter; }
  int GetLevel(LogCategories category) const { return filter.GetLevel(category); }
  void SetLevel(LogCategories categories, int level) { return filter.SetLevel(categories, level); }
  Slice<SharedString> GetComponents() { return filter.GetComponents(); }
  void SetComponents(Slice<const String> comps) { filter.SetComponents(comps); }
  void ResetFilter() { filter.ResetFilter(); }

  epEvent<> Changed;
protected:
  Logger(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  LogStream *FindLogStream(StreamRef spStream) const;

  Array<LogStream> streamList;
  LogFilter filter;
  Array<LogLine> internalLog;
  bool bEnabled = true, bLogging = false;
};

} //namespace ep

ptrdiff_t epStringify(Slice<char> buffer, String format, const LogLine &line, const epVarArg *pArgs);

#endif // EPLOGGER_H
