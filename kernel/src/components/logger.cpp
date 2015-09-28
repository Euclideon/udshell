#include "components/stream.h"

#include <time.h>

namespace ep
{
static CPropertyDesc props[] =
{
  {
    {
      "enabled", // id
      "Enabled", // displayName
      "Is Enabled", // description
    },
    &Logger::GetEnabled, // getter
    &Logger::SetEnabled, // setter
  }
};

static CMethodDesc methods[] =
{
  {
    {
      "removefilters", // id
      "Remove all logging filters", // description
    },
    &Logger::RemoveFilters, // method
  },
  {
    {
      "getfilterlevel", // id
      "Get filter level for the specified category", // description
    },
    &Logger::GetFilterLevel, // method
  },
  {
    {
      "setfilterlevel", // id
      "Filter logging for a category to the specified level", // description
    },
    &Logger::SetFilterLevel, // method
  },
  // TODO epVariant doesn't support epSlice<const epString>. fix this with wrappers?
  /*{ T
    {
      "getfiltercomponents", // id
      "Get filtered components uids", // description
    },
    &Logger::GetFilterComponents, // method
  },
  {
    {
      "setfiltercomponents", // id
      "Filter logging to the specified component UIDs", // description
    },
    &Logger::SetFilterComponents, // method
  },*/
  {
    {
      "setlevel", // id
      "Set severity level for this log stream", // description
    },
    &Logger::SetLevel, // method
  },
  {
    {
      "setcategories", // id
      "Set logging categories to output to this stream", // description
    },
    &Logger::SetCategories, // method
  },
  {
    {
      "addstream", // id
      "Add an output stream to the logger", // description
    },
    &Logger::AddStream, // method
  },
  {
    {
      "removestream", // id
      "Remove an output stream from the logger", // description
    },
    &Logger::RemoveStream, // method
  },
  {
    {
      "log", // id
      "Write a line to the log", // description
    },
    &Logger::Log, // method
  }
};

ComponentDesc Logger::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "logger", // id
  "Logger", // displayName
  "Logger", // description

  epSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  epSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr, // events
};

Logger::Logger(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  RemoveFilters();
}

Logger::LogStream *Logger::FindLogStream(StreamRef spStream) const
{
  for (auto &s : streamList)
  {
    if (s.spStream == spStream)
      return &s;
  }

  return nullptr;
}

void Logger::Log(int level, epString text, LogCategories category, epString componentUID)
{
  epMutableString<1024> out;
  char timeStr[64];

  // Check category level filter
  int catIndex;
  LogCategories tempCat = category;
  for (catIndex = 0; !(tempCat & 1); catIndex++)
    tempCat = tempCat >> 1;

  if (levelsFilter[catIndex] != -1 && levelsFilter[catIndex] < level)
    return;

  // Check component ids filter
  if (componentUID != nullptr && !componentsFilter.empty())
  {
    bool componentFound = false;
    for (epString comp : componentsFilter)
    {
      if (!comp.cmp(componentUID))
      {
        componentFound = true;
        break;
      }
    }
    if (!componentFound)
      return;
  }

  if (bLogging)
    return;
  bLogging = true;

  for (auto &s : streamList)
  {
    out = nullptr;

    if ((s.categories & category) && s.level >= level)
    {
      if (s.format & LogFormatSpecs::Timestamp)
      {
        time_t ti = time(nullptr);
#if defined(EP_WINDOWS)
        tm _tm, *pTm = &_tm;
        localtime_s(&_tm, &ti);
#else
        tm *pTm = localtime(&ti);
#endif
        strftime(timeStr, sizeof(timeStr), "[%d/%m/%d %H:%M:%S]", pTm);
        out.append((const char*)timeStr);
      }
      if (s.format & (LogFormatSpecs::Level | LogFormatSpecs::ComponentUID))
      {
        out.append("(");

        if (s.format & LogFormatSpecs::Level)
        {
          out.append(level);
          if (s.format & LogFormatSpecs::ComponentUID && componentUID != nullptr)
            out.append(", ");
        }
        if (s.format & LogFormatSpecs::ComponentUID)
          out.append(componentUID);

        out.append(")");
      }

      if (s.format & (LogFormatSpecs::Timestamp | LogFormatSpecs::Level | LogFormatSpecs::ComponentUID))
        out.append(" ");

      if (s.format & LogFormatSpecs::Category)
        out.append(category.StringOf(), ": ");

      out.append(text);

      s.spStream->WriteLn(out);
      s.spStream->Flush();
    }
  }

  bLogging = false;
}

void Logger::AddStream(StreamRef spStream, LogCategories categories, int level, LogFormatSpecs format)
{
  streamList.pushBack(LogStream(spStream, categories, level, format));
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

int Logger::SetLevel(StreamRef spStream, int level)
{
  if (LogStream *pLogStream = FindLogStream(spStream))
  {
    pLogStream->level = level;
    return 0;
  }

  // TODO Fix error checking
  return -1;
}

int Logger::GetLevel(StreamRef spStream) const
{
  if (LogStream *pLogStream = FindLogStream(spStream))
    return pLogStream->level;

  // TODO Fix error checking
  return -1;
}

int Logger::SetCategories(StreamRef spStream, LogCategories categories)
{
  if (LogStream *pLogStream = FindLogStream(spStream))
  {
    pLogStream->categories = categories;
    return 0;
  }

  // TODO Fix error checking
  return -1;
}

LogCategories Logger::GetCategories(StreamRef spStream) const
{
  if (LogStream *pLogStream = FindLogStream(spStream))
    return pLogStream->categories;

  // TODO Fix error checking
  return -1;
}

int Logger::GetFilterLevel(LogCategories category) const
{
  int catIndex;
  for (catIndex = 0; !(category & 1); catIndex++)
    category = category >> 1;

  return levelsFilter[catIndex];
}

epSlice<epSharedString> Logger::GetFilterComponents() const
{
  return componentsFilter;
}

void Logger::SetFilterLevel(LogCategories categories, int level)
{
  for (int i = 0; categories; i++)
  {
    if (categories & 1)
      levelsFilter[i] = level;

    categories = categories >> 1;
  }
}

void Logger::SetFilterComponents(epSlice<const epString> comps)
{
  componentsFilter = comps;
}

void Logger::RemoveFilters()
{
  memset(levelsFilter, -1, sizeof(levelsFilter));
  componentsFilter = nullptr;
}

} // namespace ep
