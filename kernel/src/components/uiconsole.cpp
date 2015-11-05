#include "components/uiconsole.h"
#include "components/memstream.h"
#include "components/logger.h"
#include "components/resources/buffer.h"
#include "components/shortcutmanager.h"
#include "ep/cpp/delegate.h"
#include "kernel.h"

namespace ep
{
static CPropertyDesc props[] =
{
  {
    {
      "outputsmerged", // id
      "OutputsMerged", // displayName
      "Determines whether the console and log text are output to separate text widgets or interleaved", // description
    },
    &UIConsole::GetOutputsMerged, // getter
    &UIConsole::SetOutputsMerged, // setter
  },
  {
    {
      "filtercomponents", // id
      "FilterComponents", // displayName
      "List of Components to filter the log text by", // description
    },
    &UIConsole::GetFilterComponents, // getter
    &UIConsole::SetFilterComponents, // setter
  },
  {
    {
      "filtertext", // id
      "FilterText", // displayName
      "text string to filter console and log lines by", // description
    },
    &UIConsole::GetFilterText, // getter
    &UIConsole::SetFilterText, // setter
  },
  {
    {
      "numconsolelines", // id
      "NumConsoleLines", // displayName
      "Number of console lines to output", // description
    },
    &UIConsole::GetNumConsoleLines, // getter
    nullptr,
  },
  {
    {
      "numloglines", // id
      "NumLogLines", // displayName
      "Number of log lines to output", // description
    },
    &UIConsole::GetNumLogLines, // getter
    nullptr,
  },
  {
    {
      "nummergedlines", // id
      "NumMergedLines", // displayName
      "Number of merged log + console lines to output", // description
    },
    &UIConsole::GetNumMergedLines, // getter
    nullptr,
  },
};

static CMethodDesc methods[] =
{
  {
    {
      "getfilterlevel", // id
      "Get the level filter for the given log category", // description
    },
    &UIConsole::GetFilterLevel, // method
  },
  {
    {
      "setfilterlevel", // id
      "Set the level filter for the given log category", // description
    },
    &UIConsole::SetFilterLevel, // method
  },
  {
    {
      "rebuildoutput", // id
      "Rebuild output text and send to UI", // description
    },
    &UIConsole::RebuildOutput, // method
  },
  {
    {
      "relayinput", // id
      "Send input to the Kernel's input stream", // description
    },
    &UIConsole::RelayInput, // method
  },
};

ComponentDesc UIConsole::descriptor =
{
  &UIComponent::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "uiconsole",     // id
  "UIConsole",     // displayName
  "Is a UI for a Console Panel with input and output text controls", // description

  Slice<CPropertyDesc>(props, UDARRAYSIZE(props)),   // properties
  Slice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr, // events
};

LoggerRef UIConsole::spLogger;

UIConsole::UIConsole(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : UIComponent(pType, pKernel, uid, initParams)
{
  spLogger = pKernel->GetLogger();

  // Create stream for shell output
  auto spOutBuffer = pKernel->CreateComponent<Buffer>();
  spOutBuffer->Reserve(1024);
  spOutStream = pKernel->CreateComponent<MemStream>({ {"buffer", spOutBuffer}, {"flags", OpenFlags::Write} });
  spOutStream->Changed.Subscribe(this, &UIConsole::OnStreamOutput);

  // Create stream for shell input
  auto spInBuffer = pKernel->CreateComponent<Buffer>();
  spInBuffer->Reserve(1024);
  spInStream = pKernel->CreateComponent<MemStream>({ { "buffer", spInBuffer }, { "flags", OpenFlags::Write } });

  pKernel->GetLogger()->Changed.Subscribe(this, &UIConsole::OnLogChanged);

  auto spShortcutManager = pKernel->GetShortcutManager();
  spShortcutManager->RegisterShortcut("showhideconsolewindow", "`");
  spShortcutManager->SetShortcutFunction("showhideconsolewindow", Delegate<void()>(this, &UIConsole::ToggleVisible));
}

void UIConsole::ToggleVisible()
{
  CallMethod("togglevisible");
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
        if (logFilter.FilterLogLine(*logLines[logIndex].GetLogLine()) && FilterTextLine(logLines[logIndex].text))
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
      if (logFilter.FilterLogLine(*logLines[logIndex].GetLogLine()) && FilterTextLine(logLines[logIndex].text))
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

    MutableString<0> outText;
    outText.reserve(256 * filteredMerged.length);

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
      if (logFilter.FilterLogLine(*logLines[i].GetLogLine()) && FilterTextLine(logLines[i].text))
        filteredLog.pushBack((int)i);
    }

    // TODO Change below when adding virtual scrollbar support

    MutableString<0> outText;
    outText.reserve(256 * (filteredConsole.length > filteredLog.length ? filteredConsole.length : filteredLog.length));

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
  Slice<LogLine> log = pKernel->GetLogger()->GetLog();
  LogLine &line = log.back();

  logLines.pushBack(ConsoleLine(line.ToString(), (int)log.length - 1));
  ConsoleLine &cLine = logLines.back();

  // Do filtering
  if (!logFilter.FilterLogLine(line) || !FilterTextLine(cLine.text))
    return;

  if (bOutputsMerged)
  {
    filteredMerged.pushBack(MergedLine(typeLog, (int)logLines.length - 1));
    CallMethod("appendconsoletext", (String)cLine.text);
  }
  else
  {
    filteredLog.pushBack((int)logLines.length - 1);
    CallMethod("appendlogtext", (String)cLine.text);
  }
}

void UIConsole::OnStreamOutput()
{
  int64_t newPos = spOutStream->GetPos();
  MutableString<0> buf;
  buf.reserve(size_t(newPos - pos));

  spOutStream->Seek(SeekOrigin::Begin, 0);
  Slice<void> readSlice = spOutStream->Read(buf.getBuffer());
  String readStr = (String &)readSlice;
  spOutStream->Seek(SeekOrigin::Begin, 0);

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
  auto spInBuffer = pKernel->CreateComponent<Buffer>();
  spInBuffer->Reserve(1024);
  spInBuffer->CopyBuffer(str);
  spInStream->SetBuffer(spInBuffer);
}

UIConsole::ConsoleLine::ConsoleLine(String text, int logIndex)
{
  this->text = text;
  this->logIndex = logIndex;
  if (logIndex != -1)
    ordering = spLogger->GetLog()[logIndex].ordering;
  else
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
