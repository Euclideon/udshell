#pragma once
#ifndef UI_CONSOLE_H
#define UI_CONSOLE_H

#include "ui.h"

namespace ep
{

PROTOTYPE_COMPONENT(UIConsole);
SHARED_CLASS(MemStream);

class UIConsole : public UIComponent
{
public:
  EP_COMPONENT(UIConsole);

  StreamRef GetOutStream() const { return spOutStream; }
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
  UIConsole(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  virtual ~UIConsole() {}

  void ToggleVisible();

  LogFilter logFilter;
  bool FilterLogLine(const LogLine &) const;

  void OnLogChanged();
  void OnStreamOutput();

  struct ConsoleLine
  {
    ConsoleLine(String text, int logIndex = -1);
    LogLine *GetLogLine() const
    {
      if (logIndex != -1)
        return &spLogger->GetLog()[logIndex];

      return nullptr;
    }

    SharedString text;
    double ordering;
    int logIndex;
  };

  static LoggerRef spLogger;
  MemStreamRef spInStream = nullptr;
  MemStreamRef spOutStream = nullptr;
  int64_t pos = 0;

  MutableString<256> textFilter;
  Array<ConsoleLine> consoleLines;
  Array<ConsoleLine> logLines;
  Array<int> filteredConsole;
  Array<int> filteredLog;

  enum {
    typeConsole,
    typeLog
  };
  struct MergedLine {
    MergedLine(int type, int index) : type(type), index(index) {}

    int type;
    int index;
  };
  Array<MergedLine> filteredMerged;
  bool bOutputsMerged = false;
};

} // namespace ep

#endif // UICONSOLE_H

