#include "components/logger.h"
#include "components/stream.h"
#include <time.h>

namespace ud
{
static CMethodDesc methods[] =
{
  {
    {
      "enable", // id
      "Enable output from the logger", // description
    },
    &Logger::Enable, // method
  },
  {
    {
      "disable", // id
      "Disable all output from the logger", // description
    },
    &Logger::Disable, // method
  },
  {
    {
      "set_level", // id
      "Set severity level for this log stream", // description
    },
    &Logger::SetLevel, // method
  },
  {
    {
      "set_categories", // id
      "Set logging categories to output to this stream", // description
    },
    &Logger::SetCategories, // method
  },
  {
    {
      "add_stream", // id
      "Add an output stream to the logger", // description
    },
    &Logger::AddStream, // method
  },
  {
    {
      "remove_stream", // id
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

  nullptr, // properties
  udSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr, // events
};

Logger::Logger(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  bEnabled = true;
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

int Logger::Log(int level, udString text, LogCategories category, udString componentUID)
{
  udFixedString<1024> out;
  char timeStr[64];

  for (auto &s : streamList)
  {
    out = "";

    if ((s.categories & category) && s.level >= level)
    {
      if (s.format & LogFormatSpecs::Timestamp)
      {
          time_t ti = time(nullptr);
          struct tm _tm;
          localtime_s(&_tm, &ti);
          strftime(timeStr, 64, "[%d/%m/%d %H:%M:%S]", &_tm);
          out.concat(timeStr);
      }
      if (s.format & (LogFormatSpecs::Level | LogFormatSpecs::ComponentUID))
      {
        out.concat("(");

        if (s.format & LogFormatSpecs::Level)
        {
          out = udFixedString<1024>::format("%s%d", out.toStringz(), level);
          if (s.format & LogFormatSpecs::ComponentUID && componentUID != nullptr)
            out.concat(", ");
        }
        if (s.format & LogFormatSpecs::ComponentUID)
          out.concat(componentUID);

        out.concat(")");
      }

      if (s.format & (LogFormatSpecs::Timestamp | LogFormatSpecs::Level | LogFormatSpecs::ComponentUID))
        out.concat(" ");

      if (s.format & LogFormatSpecs::Category)
        out.concat(category.StringOf(), ": ");

      out.concat(text, "\n");

      s.spStream->Write(out.toStringz(), out.length);
      s.spStream->Flush();
    }
  }

  return 0;
}

void Logger::Enable()
{
  bEnabled = true;
}

void Logger::Disable()
{
  bEnabled = false;
}

void Logger::AddStream(StreamRef spStream, LogCategories categories, int level, LogFormatSpecs format)
{
  streamList.pushBack(LogStream(spStream, categories, level, format));
}

int Logger::RemoveStream(StreamRef spStream)
{
  if (LogStream *pLogStream = FindLogStream(spStream))
  {
    streamList.removeSwapLast(*pLogStream);

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
