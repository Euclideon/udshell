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

class UIConsole : public UIComponent
{
  EP_DECLARE_COMPONENT(UIConsole, UIComponent, EPKERNEL_PLUGINVERSION, "Is a UI for a Console Panel with input and output text controls", 0)
public:

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

protected:
  UIConsole(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~UIConsole();

  void ToggleVisible(Variant::VarMap params);
  void OnLogChanged();
  void OnConsoleOutput(Slice<const void> buf);
  bool FilterLogLine(const LogLine &) const;

  LogFilter logFilter;
  bool bOutputLogToConsole = false;

  struct ConsoleLine
  {
    ConsoleLine(String text, int logIndex = -1, double ordering = 0.0);

    SharedString text;
    double ordering;
    int logIndex;
  };

  LoggerRef spLogger;
  BroadcasterRef spConsoleOut = nullptr;
  BroadcasterRef spConsoleErr = nullptr;
  BroadcasterRef spLuaOut = nullptr;

  FileRef spHistoryFile = nullptr;
  Array<SharedString> history;
  const String historyFileName = "console.history";

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

  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

} // namespace ep

#endif // UICONSOLE_H

