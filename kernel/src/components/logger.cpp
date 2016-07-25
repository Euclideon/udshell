#include "components/logger.h"
#include "ep/cpp/component/stream.h"
#include "ep/cpp/datetime.h"
#include "hal/timer.h"

#include <time.h>

namespace ep {

Array<const PropertyInfo> Logger::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY("enabled", GetEnabled, SetEnabled, "Is Enabled", nullptr, 0),
  };
}
Array<const MethodInfo> Logger::GetMethods() const
{
  return{
    EP_MAKE_METHOD(Log, "Write a line to the log"),
    EP_MAKE_METHOD(AddStream, "Add an output stream to the logger"),
    EP_MAKE_METHOD(RemoveStream, "Remove an output stream from the logger"),
    EP_MAKE_METHOD(ResetFilter, "Resets log filter to a non-filtering state"),
    EP_MAKE_METHOD(GetLevel, "Get filter level for the specified category"),
    EP_MAKE_METHOD(SetLevel, "Filter logging for a bitfield of categories to the specified level"),
    EP_MAKE_METHOD(EnableCategory, "Enables the specified category"),
    EP_MAKE_METHOD(DisableCategory, "Disables the specified category"),
    EP_MAKE_METHOD(IsCategoryEnabled, "Get enabled state for the specified category"),
    EP_MAKE_METHOD(GetComponents, "Get filtered components uids"),
    EP_MAKE_METHOD_EXPLICIT("SetComponents", SetComponents_Arr, "Filter logging to the specified component UIDs"),
    EP_MAKE_METHOD(ResetStreamFilter, "Resets a stream's log filter to a non-filtering state"),
    EP_MAKE_METHOD(GetStreamLevel, "Get a category's filter level for the given stream"),
    EP_MAKE_METHOD(SetStreamLevel, "Filter category levels for the given stream"),
    EP_MAKE_METHOD(GetStreamComponents, "Get the filtered components uids for the given stream"),
    EP_MAKE_METHOD_EXPLICIT("SetStreamComponents", SetStreamComponents_Arr, "Filter logging for the given stream to the specified component UIDs"),
  };
}
Array<const EventInfo> Logger::GetEvents() const
{
  return{
    EP_MAKE_EVENT(Changed, "Log has been updated with a new entry"),
  };
}

Logger::Logger(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

LogStream *Logger::FindLogStream(StreamRef spStream) const
{
  for (auto &s : streamList)
  {
    if (s.spStream == spStream)
      return &s;
  }

  return nullptr;
}

void Logger::Log(int level, String text, LogCategories category, String componentUID)
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
      Changed.Signal();
    }
  }

  for (int i = 0; i < numLines; i++)
  {
    LogLine &line = internalLog[internalLog.length - numLines + i];

    // Output to streams
    if(!filter.FilterLogLine(line))
    {
      bLogging = false;
      return;
    }

    for (auto &s : streamList)
    {
      if (s.filter.FilterLogLine(line))
      {
        SharedString out = line.ToString(s.format);
        s.spStream->WriteLn(out);
        s.spStream->Flush();
      }
    }
  }

  bLogging = false;
}

void Logger::AddStream(StreamRef spStream, LogFormatSpecs format)
{
  streamList.pushBack(LogStream(spStream, format, LogFilter()));
}

int Logger::RemoveStream(StreamRef spStream)
{
  if (LogStream *pLogStream = FindLogStream(spStream))
  {
    streamList.removeSwapLast(pLogStream);

    return 0;
  }

  // TODO Fix error checking
  return -1;
}

LogStream *Logger::GetLogStream(StreamRef spStream)
{
  return FindLogStream(spStream);
}

// Stream filter helper functions

int Logger::ResetStreamFilter(StreamRef spStream)
{
  if (LogStream *pLogStream = FindLogStream(spStream))
  {
    pLogStream->filter.ResetFilter();
    return 0;
  }

  return -1;
}

int Logger::GetStreamLevel(StreamRef spStream, LogCategories category) const
{
  if (LogStream *pLogStream = FindLogStream(spStream))
    return pLogStream->filter.GetLevel(category);

  // TODO Fix error checking
  return -1;
}

int Logger::SetStreamLevel(StreamRef spStream, LogCategories categories, int level)
{
  if (LogStream *pLogStream = FindLogStream(spStream))
  {
    pLogStream->filter.SetLevel(categories, level);
    return 0;
  }

  // TODO Fix error checking
  return -1;
}

Slice<SharedString> Logger::GetStreamComponents(StreamRef spStream) const
{
  if (LogStream *pLogStream = FindLogStream(spStream))
    return pLogStream->filter.GetComponents();

  // TODO Fix error checking
  return nullptr;
}

int Logger::SetStreamComponents(StreamRef spStream, Slice<const String> comps)
{
  if (LogStream *pLogStream = FindLogStream(spStream))
  {
    pLogStream->filter.SetComponents(comps);
    return 0;
  }

  // TODO Fix error checking
  return -1;
}

// LogFilter functions

int LogFilter::GetLevel(LogCategories category) const
{
  int catIndex;
  for (catIndex = 0; !(category & 1); catIndex++)
    category = category >> 1;

  return levelsFilter[catIndex];
}

void LogFilter::SetLevel(LogCategories categories, int level)
{
  for (int i = 0; categories; i++)
  {
    if (categories & 1)
      levelsFilter[i] = level;

    categories = categories >> 1;
  }
}

bool LogFilter::IsCategoryEnabled(LogCategories category) const
{
  int catIndex;
  for (catIndex = 0; !(category & 1); catIndex++)
    category = category >> 1;

  return enabledFilter[catIndex];
}

void LogFilter::EnableCategory(LogCategories categories)
{
  for (int i = 0; categories; i++)
  {
    if (categories & 1)
      enabledFilter[i] = true;

    categories = categories >> 1;
  }
}

void LogFilter::DisableCategory(LogCategories categories)
{
  for (int i = 0; categories; i++)
  {
    if (categories & 1)
      enabledFilter[i] = false;

    categories = categories >> 1;
  }
}

Slice<SharedString> LogFilter::GetComponents() const
{
  return componentsFilter;
}

void LogFilter::SetComponents(Slice<const String> comps)
{
  componentsFilter = comps;
}

void LogFilter::ResetFilter()
{
  memset(levelsFilter, -1, sizeof(levelsFilter));
  memset(enabledFilter, true, sizeof(enabledFilter));
  componentsFilter = nullptr;
}

bool LogFilter::FilterLogLine(LogLine &line) const
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
  SharedString out = line.ToString(LogDefaults::Format);

  // if we're only counting
  if (!buffer.ptr)
    return out.length;

  // if the buffer is too small
  if (buffer.length < out.length)
    return buffer.length - out.length;

  out.copyTo(buffer);

  return out.length;
}

SharedString LogLine::ToString(LogFormatSpecs format) const
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
