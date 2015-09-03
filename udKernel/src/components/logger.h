#pragma once
#ifndef UD_LOGGER_H
#define UD_LOGGER_H

#include "component.h"

namespace ud
{
PROTOTYPE_COMPONENT(Logger);
SHARED_CLASS(Stream);

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
  Level = 5,
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

  void AddStream(StreamRef spStream, LogCategories categories = LogDefaults::Categories, int level = LogDefaults::Level, LogFormatSpecs format = LogDefaults::Format);
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
