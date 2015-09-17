#include "components/stream.h"

#include <time.h>

namespace ud
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

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "logger", // id
  "Logger", // displayName
  "Logger", // description

  udSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  udSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr, // events
};

Logger::Logger(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{}

Logger::LogStream *Logger::FindLogStream(StreamRef spStream) const
{
  for (auto &s : streamList)
  {
    if (s.spStream == spStream)
      return &s;
  }

  return nullptr;
}

int Logger::Log(int level, udString text, LogCategories category, udString componentUID)
{
  udMutableString<1024> out;
  char timeStr[64];

  if (bLogging)
    return -1; // TODO Handle errors
  bLogging = true;

  for (auto &s : streamList)
  {
    out = nullptr;

    if ((s.categories & category) && s.level >= level)
    {
      if (s.format & LogFormatSpecs::Timestamp)
      {
        time_t ti = time(nullptr);
#if UDPLATFORM_WINDOWS
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
  return 0;
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

int Logger::AddCategory(StreamRef spStream, LogCategories category)
{
  if (LogStream *pLogStream = FindLogStream(spStream))
  {
    pLogStream->categories &= category;
    return 0;
  }

  // TODO Fix error checking
  return -1;
}

int Logger::RemoveCategory(StreamRef spStream, LogCategories category)
{
  if (LogStream *pLogStream = FindLogStream(spStream))
  {
    pLogStream->categories &= ~category;
    return 0;
  }

  // TODO Fix error checking
  return -1;
}

} // namespace ud
