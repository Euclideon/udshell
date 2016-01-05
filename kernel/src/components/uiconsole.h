#pragma once
#ifndef UI_CONSOLE_H
#define UI_CONSOLE_H

#include "ui.h"
#include "logger.h"

namespace ep {

SHARED_CLASS(UIConsole);
SHARED_CLASS(MemStream);
SHARED_CLASS(Broadcaster);
SHARED_CLASS(Stream);
SHARED_CLASS(Activity);

class UIConsole : public UIComponent
{
  EP_DECLARE_COMPONENT(UIConsole, UIComponent, EPKERNEL_PLUGINVERSION, "Is a UI for a Console Panel with input and output text controls")
public:

  StreamRef GetInStream() const { return spInStream; }

  void RelayInput(String str);

  int GetNumConsoleLines() const { return (int)filteredConsole.length; }
  int GetNumLogLines() const { return (int)filteredLog.length; }
  int GetNumMergedLines() const { return (int)filteredMerged.length; }

  bool GetOutputsMerged() const { return bOutputsMerged; }
  void SetOutputsMerged(bool bMerged)
  {
    bOutputsMerged = bMerged;
    RebuildOutput();
  }

  void RebuildOutput();

  LogFilter GetFilter() const { return logFilter; }
  void SetFilter(LogFilter filter)
  {
    logFilter = filter;
    RebuildOutput();
  };

  // Filter getter/setter helper functions
  int GetFilterLevel(LogCategories category) const { return logFilter.GetLevel(category); }
  void SetFilterLevel(LogCategories categories, int level)
  {
    logFilter.SetLevel(categories, level);
    RebuildOutput();
  }

  String GetFilterComponents() const;
  void SetFilterComponents(String str);

  bool FilterTextLine(String line) const;
  String GetFilterText() const { return textFilter; }
  void SetFilterText(String str)
  {
    textFilter = str;
    RebuildOutput();
  }

protected:
  UIConsole(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~UIConsole() {}

  void ToggleVisible(Variant::VarMap params);

  LogFilter logFilter;
  bool FilterLogLine(const LogLine &) const;

  void OnLogChanged();
  void OnConsoleOutput(Slice<const void> buf);

  struct ConsoleLine
  {
    ConsoleLine(String text, int logIndex = -1, double ordering = 0.0);

    SharedString text;
    double ordering;
    int logIndex;
  };

  LoggerRef spLogger;
  MemStreamRef spInStream = nullptr;
  BroadcasterRef spConsoleOut = nullptr;
  BroadcasterRef spConsoleErr = nullptr;

  int64_t pos = 0;

  MutableString<256> textFilter;
  Array<ConsoleLine> consoleLines;
  Array<ConsoleLine> logLines;
  Array<int> filteredConsole;
  Array<int> filteredLog;

  enum
  {
    typeConsole,
    typeLog
  };
  struct MergedLine
  {
    MergedLine(int type, int index) : type(type), index(index) {}

    int type;
    int index;
  };
  Array<MergedLine> filteredMerged;
  bool bOutputsMerged = false;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(OutputsMerged, "Determines whether the console and log text are output to separate text widgets or interleaved", nullptr, 0),
      EP_MAKE_PROPERTY(FilterComponents, "List of Components to filter the log text by", nullptr, 0),
      EP_MAKE_PROPERTY(FilterText, "Text string to filter console and log lines by", nullptr, 0),
      EP_MAKE_PROPERTY_RO(NumConsoleLines, "Number of console lines to output", nullptr, 0),
      EP_MAKE_PROPERTY_RO(NumLogLines, "Number of log lines to output", nullptr, 0),
      EP_MAKE_PROPERTY_RO(NumMergedLines, "Number of merged log + console lines to output", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(GetFilterLevel, "Get the level filter for the given log category"),
      EP_MAKE_METHOD(SetFilterLevel, "Set the level filter for the given log category"),
      EP_MAKE_METHOD(RebuildOutput, "Rebuild output text and send to UI"),
      EP_MAKE_METHOD(RelayInput, "Send input to the Kernel's input stream"),
    };
  }
};

} // namespace ep

#endif // UICONSOLE_H

