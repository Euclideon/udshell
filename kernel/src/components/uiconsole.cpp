#include "components/uiconsole.h"
#include "components/memstream.h"
#include "components/broadcaster.h"
#include "components/logger.h"
#include "components/resources/buffer.h"
#include "components/commandmanager.h"
#include "ep/cpp/delegate.h"
#include "kernel.h"

namespace ep {

UIConsole::UIConsole(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : UIComponent(pType, pKernel, uid, initParams)
{
  spConsoleOut = pKernel->GetStdOutBroadcaster();
  spConsoleOut->Written.Subscribe(this, &UIConsole::OnConsoleOutput);
  spConsoleErr = pKernel->GetStdErrBroadcaster();
  spConsoleErr->Written.Subscribe(this, &UIConsole::OnConsoleOutput);

  // Create stream for shell input
  auto spInBuffer = pKernel->CreateComponent<Buffer>();
  spInBuffer->Reserve(1024);
  spInStream = pKernel->CreateComponent<MemStream>({ { "buffer", spInBuffer }, { "flags", OpenFlags::Write } });

  spLogger = ((kernel::Kernel*)pKernel)->GetLogger();
  ((kernel::Kernel*)pKernel)->GetLogger()->Changed.Subscribe(this, &UIConsole::OnLogChanged);

  auto spCommandManager = pKernel->GetCommandManager();
  spCommandManager->RegisterCommand("showhideconsolewindow", Delegate<void(Variant::VarMap)>(this, &UIConsole::ToggleVisible), "", "", "`");
}

void UIConsole::ToggleVisible(Variant::VarMap params)
{
  CallMethod("togglevisible", nullptr);
}

void UIConsole::RebuildOutput()
{
  filteredConsole = nullptr;
  filteredLog = nullptr;
  filteredMerged = nullptr;

  if (bOutputsMerged)
  {
    size_t logIndex = 0, consoleIndex = 0;
    while (logIndex < logLines.length && consoleIndex < consoleLines.length)
    {
      if (logLines[logIndex].ordering < consoleLines[consoleIndex].ordering)
      {
        if (logFilter.FilterLogLine(*spLogger->GetLogLine(logLines[logIndex].logIndex)) && FilterTextLine(logLines[logIndex].text))
          filteredMerged.pushBack(MergedLine(typeLog, (int)logIndex));
        logIndex++;
      }
      else
      {
        if(FilterTextLine(consoleLines[consoleIndex].text))
          filteredMerged.pushBack(MergedLine(typeConsole, (int)consoleIndex));
        consoleIndex++;
      }
    }
    while (logIndex < logLines.length)
    {
      if (logFilter.FilterLogLine(*spLogger->GetLogLine(logLines[logIndex].logIndex)) && FilterTextLine(logLines[logIndex].text))
        filteredMerged.pushBack(MergedLine(typeLog, (int)logIndex));
      logIndex++;
    }
    while (consoleIndex < consoleLines.length)
    {
      if (FilterTextLine(consoleLines[consoleIndex].text))
        filteredMerged.pushBack(MergedLine(typeConsole, (int)consoleIndex));
      consoleIndex++;
    }

    // TODO Change below when adding virtual scrollbar support

    MutableString<0> outText(Reserve, 256 * filteredMerged.length);

    for (MergedLine &m : filteredMerged)
    {
      if (m.type == typeConsole)
        outText.append(consoleLines[m.index].text, "\n");
      else
        outText.append(logLines[m.index].text, "\n");
    }

    if (!outText.empty() && outText.back() == '\n')
      outText.length--;

    CallMethod("setconsoletext", (String)outText);
  }
  else
  {
    for (size_t i = 0; i < consoleLines.length; i++)
    {
      if (FilterTextLine(consoleLines[i].text))
        filteredConsole.pushBack((int)i);
    }

    for (size_t i = 0; i < logLines.length; i++)
    {
      if (logFilter.FilterLogLine(*spLogger->GetLogLine(logLines[i].logIndex)) && FilterTextLine(logLines[i].text))
        filteredLog.pushBack((int)i);
    }

    // TODO Change below when adding virtual scrollbar support

    MutableString<0> outText(Reserve, 256 * (filteredConsole.length > filteredLog.length ? filteredConsole.length : filteredLog.length));

    for (int i : filteredConsole)
      outText.append(consoleLines[i].text, "\n");

    if (!outText.empty() && outText.back() == '\n')
      outText.length--;

    CallMethod("setconsoletext", (String)outText);

    outText.length = 0;

    for (int i : filteredLog)
      outText.append(logLines[i].text, "\n");

    if (!outText.empty() && outText.back() == '\n')
      outText.length--;

    CallMethod("setlogtext", (String)outText);
  }
}

void UIConsole::OnLogChanged()
{
  Slice<LogLine> log = ((kernel::Kernel*)pKernel)->GetLogger()->GetLog();
  LogLine &line = log.back();

  logLines.pushBack(ConsoleLine(line.ToString(), (int)log.length - 1, spLogger->GetLogLine((int)log.length - 1)->ordering));
  ConsoleLine &cLine = logLines.back();

  // Do filtering
  if (!logFilter.FilterLogLine(line) || !FilterTextLine(cLine.text))
    return;

  if (bOutputsMerged)
  {
    filteredMerged.pushBack(MergedLine(typeLog, (int)logLines.length - 1));
    CallMethod("appendconsoletext", cLine.text);
  }
  else
  {
    filteredLog.pushBack((int)logLines.length - 1);
    CallMethod("appendlogtext", cLine.text);
  }
}

void UIConsole::OnConsoleOutput(Slice<const void> buf)
{
  String readStr = (String &)buf;

  while (!readStr.empty())
  {
    String token = readStr.popToken("\n");
    if (!token.empty())
    {
      consoleLines.pushBack(ConsoleLine(token));
      if (FilterTextLine(token))
      {
        if (bOutputsMerged)
          filteredConsole.pushBack((int)consoleLines.length - 1);
        else
          filteredMerged.pushBack(MergedLine(typeConsole, (int)consoleLines.length - 1));

        CallMethod("appendconsoletext", token);
      }
    }
  }
}

void UIConsole::RelayInput(String str)
{
  pKernel->Exec(str);
}

UIConsole::ConsoleLine::ConsoleLine(String text, int logIndex, double ordering)
{
  this->text = text;
  this->logIndex = logIndex;
  if (ordering == 0.0)
    ordering = epPerformanceCounter();
}

// Filter getter/setter helper functions

String UIConsole::GetFilterComponents() const
{
  MutableString<1024> str;

  Array<SharedString> comps = logFilter.GetComponents();
  for (SharedString &c: comps)
  {
    if (c.length == 0)
      str.append("$;");
    else
      str.append(c, ";");
  }

  if (str.length)
    str.popBack();

  return str;
}

void UIConsole::SetFilterComponents(String str)
{
  String token;
  Array<String> comps;

  while (!str.empty())
  {
    token = str.popToken(";, ");
    if (!token.empty())
    {
      if (token[0] == '$')
        comps.pushBack("");
      else
        comps.pushBack(token);
    }
  }

  logFilter.SetComponents(comps);
  RebuildOutput();
}

bool UIConsole::FilterTextLine(String line) const
{
  if (!textFilter.empty() && line.findFirstIC(textFilter) == line.length)
    return false;

  return true;
}

}
