#pragma once
#ifndef EP_CONSOLE_H
#define EP_CONSOLE_H

#include "ep/cpp/component/memstream.h"
#include "logger.h"
#include "ep/cpp/component/file.h"

namespace ep {

SHARED_CLASS(Console);
SHARED_CLASS(MemStream);
SHARED_CLASS(Broadcaster);
SHARED_CLASS(Activity);
SHARED_CLASS(File);

class Console : public Component
{
  EP_DECLARE_COMPONENT(ep, Console, Component, EPKERNEL_PLUGINVERSION, "Console logging data component", 0)
public:

  void addBroadcaster(BroadcasterRef spBC);
  void removeBroadcaster(BroadcasterRef spBC);
  bool isLogOutputEnabled();

  void relayInput(String str);

  int getNumConsoleLines() const { return (int)filteredConsole.length; }

  void rebuildOutput();

  LogFilter getFilter() const { return logFilter; }
  void setFilter(LogFilter filter)
  {
    logFilter = filter;
    rebuildOutput();
  };

  // Filter getter/setter helper functions
  int getFilterLevel(LogCategories category) const { return logFilter.getLevel(category); }
  void setFilterLevel(LogCategories categories, int level)
  {
    logFilter.setLevel(categories, level);
    rebuildOutput();
  }

  bool isFilterCategoryEnabled(LogCategories category) const { return logFilter.isCategoryEnabled(category); }
  void enableFilterCategory(LogCategories categories) { logFilter.enableCategory(categories); rebuildOutput(); }
  void disableFilterCategory(LogCategories categories) { logFilter.disableCategory(categories); rebuildOutput(); }

  String getFilterComponents() const;
  void setFilterComponents(String str);

  bool filterTextLine(String line) const;
  String getFilterText() const { return textFilter; }
  void setFilterText(String str)
  {
    textFilter = str;
    rebuildOutput();
  }

  // History
  String getHistoryLine(size_t lineNumber) const { return history[lineNumber]; }
  void appendHistory(String str);
  size_t getHistoryLength() const { return history.length; }

  bool outputLog() const { return bOutputLog; }
  bool hasInput() const { return bHasInput; }
  String getTitle() const { return title; }

protected:
  Console(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~Console();

  //void ToggleVisible(Variant::VarMap params);
  void onLogChanged();
  void onConsoleOutput(Slice<const void> buf);
  bool filterLogLine(const LogLine &) const;

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

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

} // namespace ep

#endif // EP_CONSOLE_H

