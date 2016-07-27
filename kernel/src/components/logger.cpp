#include "components/logger.h"
#include "ep/cpp/component/stream.h"
#include "ep/cpp/datetime.h"
#include "hal/timer.h"

#include <time.h>

namespace ep {

Array<const PropertyInfo> Logger::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("enabled", getEnabled, setEnabled, "Is Enabled", nullptr, 0),
  };
}
Array<const MethodInfo> Logger::getMethods() const
{
  return{
    EP_MAKE_METHOD(log, "Write a line to the log"),
    EP_MAKE_METHOD(addStream, "Add an output stream to the logger"),
    EP_MAKE_METHOD(removeStream, "Remove an output stream from the logger"),
    EP_MAKE_METHOD(resetFilter, "Resets log filter to a non-filtering state"),
    EP_MAKE_METHOD(getLevel, "Get filter level for the specified category"),
    EP_MAKE_METHOD(setLevel, "Filter logging for a bitfield of categories to the specified level"),
    EP_MAKE_METHOD(enableCategory, "Enables the specified category"),
    EP_MAKE_METHOD(disableCategory, "Disables the specified category"),
    EP_MAKE_METHOD(isCategoryEnabled, "Get enabled state for the specified category"),
    EP_MAKE_METHOD(getComponents, "Get filtered components uids"),
    EP_MAKE_METHOD_EXPLICIT("SetComponents", setComponents_Arr, "Filter logging to the specified component UIDs"),
    EP_MAKE_METHOD(resetStreamFilter, "Resets a stream's log filter to a non-filtering state"),
    EP_MAKE_METHOD(getStreamLevel, "Get a category's filter level for the given stream"),
    EP_MAKE_METHOD(setStreamLevel, "Filter category levels for the given stream"),
    EP_MAKE_METHOD(getStreamComponents, "Get the filtered components uids for the given stream"),
    EP_MAKE_METHOD_EXPLICIT("SetStreamComponents", setStreamComponents_Arr, "Filter logging for the given stream to the specified component UIDs"),
  };
}
Array<const EventInfo> Logger::getEvents() const
{
  return{
    EP_MAKE_EVENT(changed, "Log has been updated with a new entry"),
  };
}

Logger::Logger(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

LogStream *Logger::findLogStream(StreamRef spStream) const
{
  for (auto &s : streamList)
  {
    if (s.spStream == spStream)
      return &s;
  }

  return nullptr;
}

void Logger::log(int level, String text, LogCategories category, String componentUID)
{
  if (bLogging  || !bEnabled)
    return;
  bLogging = true;

  int numLines = 0;

  // Add log line to the internal log
  while (!text.empty())
  {
    String token = text.popToken("\n");
    if (!token.empty())
    {
      numLines++;
      internalLog.pushBack(LogLine(level, token, category, componentUID));
      changed.signal();
    }
  }

  for (int i = 0; i < numLines; i++)
  {
    LogLine &line = internalLog[internalLog.length - numLines + i];

    // Output to streams
    if(!filter.filterLogLine(line))
    {
      bLogging = false;
      return;
    }

    for (auto &s : streamList)
    {
      if (s.filter.filterLogLine(line))
      {
        SharedString out = line.toString(s.format);
        s.spStream->writeLn(out);
        s.spStream->flush();
      }
    }
  }

  bLogging = false;
}

void Logger::addStream(StreamRef spStream, LogFormatSpecs format)
{
  streamList.pushBack(LogStream(spStream, format, LogFilter()));
}

int Logger::removeStream(StreamRef spStream)
{
  if (LogStream *pLogStream = findLogStream(spStream))
  {
    streamList.removeSwapLast(pLogStream);

    return 0;
  }

  // TODO Fix error checking
  return -1;
}

LogStream *Logger::getLogStream(StreamRef spStream)
{
  return findLogStream(spStream);
}

// Stream filter helper functions

int Logger::resetStreamFilter(StreamRef spStream)
{
  if (LogStream *pLogStream = findLogStream(spStream))
  {
    pLogStream->filter.resetFilter();
    return 0;
  }

  return -1;
}

int Logger::getStreamLevel(StreamRef spStream, LogCategories category) const
{
  if (LogStream *pLogStream = findLogStream(spStream))
    return pLogStream->filter.getLevel(category);

  // TODO Fix error checking
  return -1;
}

int Logger::setStreamLevel(StreamRef spStream, LogCategories categories, int level)
{
  if (LogStream *pLogStream = findLogStream(spStream))
  {
    pLogStream->filter.setLevel(categories, level);
    return 0;
  }

  // TODO Fix error checking
  return -1;
}

Slice<SharedString> Logger::getStreamComponents(StreamRef spStream) const
{
  if (LogStream *pLogStream = findLogStream(spStream))
    return pLogStream->filter.getComponents();

  // TODO Fix error checking
  return nullptr;
}

int Logger::setStreamComponents(StreamRef spStream, Slice<const String> comps)
{
  if (LogStream *pLogStream = findLogStream(spStream))
  {
    pLogStream->filter.setComponents(comps);
    return 0;
  }

  // TODO Fix error checking
  return -1;
}

// LogFilter functions

int LogFilter::getLevel(LogCategories category) const
{
  int catIndex;
  for (catIndex = 0; !(category & 1); catIndex++)
    category = category >> 1;

  return levelsFilter[catIndex];
}

void LogFilter::setLevel(LogCategories categories, int level)
{
  for (int i = 0; categories; i++)
  {
    if (categories & 1)
      levelsFilter[i] = level;

    categories = categories >> 1;
  }
}

bool LogFilter::isCategoryEnabled(LogCategories category) const
{
  int catIndex;
  for (catIndex = 0; !(category & 1); catIndex++)
    category = category >> 1;

  return enabledFilter[catIndex];
}

void LogFilter::enableCategory(LogCategories categories)
{
  for (int i = 0; categories; i++)
  {
    if (categories & 1)
      enabledFilter[i] = true;

    categories = categories >> 1;
  }
}

void LogFilter::disableCategory(LogCategories categories)
{
  for (int i = 0; categories; i++)
  {
    if (categories & 1)
      enabledFilter[i] = false;

    categories = categories >> 1;
  }
}

Slice<SharedString> LogFilter::getComponents() const
{
  return componentsFilter;
}

void LogFilter::setComponents(Slice<const String> comps)
{
  componentsFilter = comps;
}

void LogFilter::resetFilter()
{
  memset(levelsFilter, -1, sizeof(levelsFilter));
  memset(enabledFilter, true, sizeof(enabledFilter));
  componentsFilter = nullptr;
}

bool LogFilter::filterLogLine(LogLine &line) const
{
  // Check category level filter
  int catIndex;
  LogCategories tempCat = line.category;
  for (catIndex = 0; !(tempCat & 1); catIndex++)
    tempCat = tempCat >> 1;

  if (enabledFilter[catIndex] == false || (levelsFilter[catIndex] != -1 && levelsFilter[catIndex] < line.level))
    return false;

  // Check component ids filter
  if (!componentsFilter.empty())
  {
    bool componentFound = false;
    for (String comp : componentsFilter)
    {
      if (!comp.cmp(line.componentUID))
      {
        componentFound = true;
        break;
      }
    }
    if (!componentFound)
      return false;
  }

  return true;
}

LogLine::LogLine(int level, SharedString text, LogCategories category, SharedString componentID) :
    level(level), category(category)
{
  timestamp = time(nullptr);
  ordering = epPerformanceCounter();

  this->text = text;
  this->componentUID = componentUID;
}

ptrdiff_t epStringify(Slice<char> buffer, String epUnusedParam(format), const LogLine &line, const VarArg *epUnusedParam(pArgs))
{
  SharedString out = line.toString(LogDefaults::Format);

  // if we're only counting
  if (!buffer.ptr)
    return out.length;

  // if the buffer is too small
  if (buffer.length < out.length)
    return buffer.length - out.length;

  out.copyTo(buffer);

  return out.length;
}

SharedString LogLine::toString(LogFormatSpecs format) const
{
  MutableString256 out;
  char timeStr[64];

#if defined(EP_WINDOWS)
  tm _tm, *pTm = &_tm;
  localtime_s(&_tm, &timestamp);
#else
  tm *pTm = localtime(&timestamp);
#endif
  strftime(timeStr, sizeof(timeStr), "[%Y-%m-%d %H:%M:%S]", pTm);

  out.format("{0}{1}{2,?10}{3}{4}{5}{6}{7}{8}{9}",
    ((format & LogFormatSpecs::Timestamp) ? (const char*)timeStr : ""),
    ((format & (LogFormatSpecs::Level | LogFormatSpecs::ComponentUID)) ? "(" : ""),
    level,
    ((format & (LogFormatSpecs::ComponentUID | LogFormatSpecs::Level)) && componentUID != nullptr ? ", " : ""),
    ((format & LogFormatSpecs::ComponentUID) ? componentUID : ""),
    ((format & (LogFormatSpecs::Level | LogFormatSpecs::ComponentUID)) ? ")" : ""),
    ((format & (LogFormatSpecs::Timestamp | LogFormatSpecs::Level | LogFormatSpecs::ComponentUID)) ? " " : ""),
    ((format & LogFormatSpecs::Category) ? category.StringOf() : ""),
    ((format & LogFormatSpecs::Category) ? ": " : ""),
    text,
    format & LogFormatSpecs::Level
    );

  return out;
}

} // namespace ep
