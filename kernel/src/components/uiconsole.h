#pragma once
#ifndef UI_CONSOLE_H
#define UI_CONSOLE_H

#include "ep/cpp/component/uicomponent.h"
#include "memstream.h"
#include "logger.h"
#include "file.h"

namespace ep {

SHARED_CLASS(UIConsole);
SHARED_CLASS(MemStream);
SHARED_CLASS(Broadcaster);
SHARED_CLASS(Activity);
SHARED_CLASS(File);

class UIConsole : public Component
{
  EP_DECLARE_COMPONENT(UIConsole, Component, EPKERNEL_PLUGINVERSION, "Is a UI for a Console Panel Tab with input and output text controls", 0)
public:

  void AddBroadcaster(BroadcasterRef spBC);
  void RemoveBroadcaster(BroadcasterRef spBC);
  bool IsLogOutputEnabled();

  void RelayInput(String str);

  int GetNumConsoleLines() const { return (int)filteredConsole.length; }

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

  bool IsFilterCategoryEnabled(LogCategories category) const { return logFilter.IsCategoryEnabled(category); }
  void EnableFilterCategory(LogCategories categories) { logFilter.EnableCategory(categories); RebuildOutput(); }
  void DisableFilterCategory(LogCategories categories) { logFilter.DisableCategory(categories); RebuildOutput(); }

  String GetFilterComponents() const;
  void SetFilterComponents(String str);

  bool FilterTextLine(String line) const;
  String GetFilterText() const { return textFilter; }
  void SetFilterText(String str)
  {
    textFilter = str;
    RebuildOutput();
  }

  // History
  String GetHistoryLine(size_t lineNumber) const { return history[lineNumber]; }
  void AppendHistory(String str);
  size_t GetHistoryLength() const { return history.length; }

  bool OutputLog() const { return bOutputLog; }
  bool HasInput() const { return bHasInput; }
  String GetTitle() const { return title; }

protected:
  UIConsole(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~UIConsole();

  //void ToggleVisible(Variant::VarMap params);
  void OnLogChanged();
  void OnConsoleOutput(Slice<const void> buf);
  bool FilterLogLine(const LogLine &) const;

  LogFilter logFilter;
  bool bOutputLogToConsole = false;

  SharedString title;
  bool bHasInput;
  Delegate<void(String)> inputFunc;
  Delegate<void(String)> setOutputFunc;
  Delegate<void(String)> appendOutputFunc;
  bool bOutputLog;
  Array<BroadcasterRef> outputBCArray;

  struct ConsoleLine
  {
    ConsoleLine(String text, int logIndex = -1, double ordering = 0.0);

    SharedString text;
    double ordering;
    int logIndex;
  };

  LoggerRef spLogger;

  FileRef spHistoryFile = nullptr;
  Array<SharedString> history;
  SharedString historyFileName;

  MutableString<256> textFilter;
  Array<ConsoleLine> consoleLines;
  Array<int> filteredConsole;

  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

} // namespace ep

#endif // UICONSOLE_H

