#pragma once
#ifndef EP_LOGGER_H
#define EP_LOGGER_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/stream.h"
namespace ep
{

SHARED_CLASS(Logger);

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

  SharedString toString(LogFormatSpecs format = LogDefaults::Format) const;

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
  int getLevel(LogCategories category) const;
  void setLevel(LogCategories categories, int level);
  void enableCategory(LogCategories categories);
  void disableCategory(LogCategories categories);
  bool isCategoryEnabled(LogCategories category) const;
  Slice<SharedString> getComponents() const;
  void setComponents(Slice<const String> comps);
  void resetFilter();
  bool filterLogLine(LogLine &line) const;

  LogFilter()
  {
    resetFilter();
  }
protected:
  int levelsFilter[NUM_LOG_CATEGORIES];
  bool enabledFilter[NUM_LOG_CATEGORIES];
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
  EP_DECLARE_COMPONENT(ep, Logger, Component, EPKERNEL_PLUGINVERSION, "Logger desc...", 0)
public:

  void log(int level, String text, LogCategories category = LogCategories::Debug, String componentUID = nullptr);

  bool getEnabled() const { return bEnabled; }
  void setEnabled(bool bEnable) { this->bEnabled = bEnable; }

  const Slice<LogLine> getLog() const { return internalLog; }
  LogLine *getLogLine(int index) const
  {
    if (index >= 0 && index < (int)internalLog.length)
      return &internalLog[index];
    else
      return nullptr;
  }

  void addStream(StreamRef spStream, LogFormatSpecs format = LogDefaults::Format);
  int removeStream(StreamRef spStream);

  // Stream helper functions
  LogStream *getLogStream(StreamRef spStream);
  // Stream level filtering
  int getStreamLevel(StreamRef spStream, LogCategories category) const;
  int setStreamLevel(StreamRef spStream, LogCategories categories, int level);
  Slice<SharedString> getStreamComponents(StreamRef spStream) const;
  int setStreamComponents(StreamRef spStream, Slice<const String> comps);
  int resetStreamFilter(StreamRef spStream);

  // Logger level filtering
  LogFilter &getFilter() { return filter; }
  int getLevel(LogCategories category) const { return filter.getLevel(category); }
  void setLevel(LogCategories categories, int level) { filter.setLevel(categories, level); }
  void enableCategory(LogCategories categories) { filter.enableCategory(categories); }
  void disableCategory(LogCategories categories) { filter.disableCategory(categories); }
  bool isCategoryEnabled(LogCategories category) const { return filter.isCategoryEnabled(category); }
  Slice<SharedString> getComponents() const { return filter.getComponents(); }
  void setComponents(Slice<const String> comps) { filter.setComponents(comps); }
  void resetFilter() { filter.resetFilter(); }

  // TODO: Move these to the protected section once the refactor of Methods/Properties in Component is done.
  void setComponents_Arr(const Array<const String> &comps) { setComponents(comps); }
  int setStreamComponents_Arr(StreamRef spStream, const Array<const String> &comps) { return setStreamComponents(spStream, comps); }

  Event<> changed;
protected:
  Logger(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~Logger() { bEnabled = false; }
  LogStream *findLogStream(StreamRef spStream) const;

  Array<LogStream> streamList;
  LogFilter filter;
  Array<LogLine> internalLog;
  bool bEnabled = true, bLogging = false;

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
  Array<const EventInfo> getEvents() const;
};

ptrdiff_t epStringify(Slice<char> buffer, String format, const LogLine &line, const VarArg *pArgs);

} //namespace ep

#endif // EPLOGGER_H
