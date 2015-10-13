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

  void UIConsole::RelayInput(epString str);

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

  epString GetFilterComponents() const;
  void SetFilterComponents(epString str);

  bool FilterTextLine(epString line) const;
  epString GetFilterText() const { return textFilter; }
  void SetFilterText(epString str)
  {
    textFilter = str;
    RebuildOutput();
  }

protected:
  UIConsole(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);
  virtual ~UIConsole() {}

  void ToggleVisible();

  LogFilter logFilter;
  bool FilterLogLine(const LogLine &) const;

  void OnLogChanged();
  void OnStreamOutput();

  struct ConsoleLine
  {
    ConsoleLine(epString text, int logIndex = -1);
    LogLine *GetLogLine() const
    {
      if (logIndex != -1)
        return &spLogger->GetLog()[logIndex];

      return nullptr;
    }

    epSharedString text;
    double ordering;
    int logIndex;
  };

  static LoggerRef spLogger;
  MemStreamRef spInStream = nullptr;
  MemStreamRef spOutStream = nullptr;
  int64_t pos = 0;
  epArray<ConsoleLine> consoleLines;
  epArray<ConsoleLine> logLines;
  bool bOutputsMerged = false;
  epMutableString<256> textFilter;
};

} // namespace ep

#endif // UICONSOLE_H

