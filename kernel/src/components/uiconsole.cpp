#include "components/uiconsole.h"
#include "components/memstream.h"
#include "components/logger.h"
#include "components/resources/buffer.h"
#include "components/shortcutmanager.h"
#include "ep/epdelegate.h"
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
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "uiconsole",     // id
  "UIConsole",     // displayName
  "Is a UI for a Console Panel with input and output text controls", // description

  epSlice<CPropertyDesc>(props, UDARRAYSIZE(props)),   // properties
  epSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr, // events
};

UIConsole::UIConsole(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
  : UIComponent(pType, pKernel, uid, initParams)
{
  // Create stream for shell output
  auto spOutBuffer = pKernel->CreateComponent<Buffer>();
  spOutBuffer->Allocate(1024);
  spOutStream = pKernel->CreateComponent<MemStream>({ {"buffer", spOutBuffer}, {"flags", OpenFlags::Write} });
  spOutStream->Changed.Subscribe(this, &UIConsole::OnStreamOutput);

  // Create stream for shell input
  auto spInBuffer = pKernel->CreateComponent<Buffer>();
  spInBuffer->Allocate(1024);
  spInStream = pKernel->CreateComponent<MemStream>({ { "buffer", spInBuffer }, { "flags", OpenFlags::Write } });

  pKernel->GetLogger()->Changed.Subscribe(this, &UIConsole::OnLogChanged);

  auto spShortcutManager = pKernel->GetShortcutManager();
  spShortcutManager->RegisterShortcut("showhideconsolewindow", "`");
  spShortcutManager->SetShortcutFunction("showhideconsolewindow", epDelegate<void()>(this, &UIConsole::ToggleVisible));
}

// Change this once QML bindings support passing a QML function to C++?
void UIConsole::ToggleVisible()
{
  CallMethod("togglevisible");
}

void UIConsole::RebuildOutput()
{
  epSlice<LogLine> log = pKernel->GetLogger()->GetLog();
  epMutableString<256> lineStr;

  if (bOutputsMerged)
  {
    epMutableString<0> outText;
    outText.reserve(256 * (log.length + consoleLines.length));

    int logIndex = 0, consoleIndex = 0;
    while (logIndex < log.length && consoleIndex < consoleLines.length)
    {
      if (log[logIndex].ordering < consoleLines[consoleIndex].ordering)
      {
        if (logFilter.FilterLogLine(log[logIndex]))
        {
          lineStr = log[logIndex].ToString();
          if(FilterTextLine(lineStr))
            outText.append(lineStr, "\n");
        }
        logIndex++;
      }
      else
      {
        if(FilterTextLine(consoleLines[consoleIndex].text))
          outText.append(consoleLines[consoleIndex].text, "\n");
        consoleIndex++;
      }
    }
    while (logIndex < log.length)
    {
      if (logFilter.FilterLogLine(log[logIndex]))
      {
        lineStr = log[logIndex].ToString();
        if (FilterTextLine(lineStr))
          outText.append(lineStr, "\n");
      }
      logIndex++;
    }
    while (consoleIndex < consoleLines.length)
    {
      if (FilterTextLine(consoleLines[consoleIndex].text))
        outText.append(consoleLines[consoleIndex].text, "\n");
      consoleIndex++;
    }

    if (outText.back() == '\n')
      outText.length--;

    CallMethod("setconsoletext", (epString)outText);
  }
  else
  {
    epMutableString<0> outText;
    outText.reserve(256 * (consoleLines.length > log.length ? consoleLines.length : log.length));

    for(int i = 0; i < consoleLines.length; i++)
    {
      if (FilterTextLine(consoleLines[i].text))
        outText.append(consoleLines[i].text, "\n");
    }

    if (outText.back() == '\n')
      outText.length--;

    CallMethod("setconsoletext", (epString)outText);

    outText.length = 0;
    for (int i = 0; i < log.length; i++)
    {
      if (logFilter.FilterLogLine(log[i]))
      {
        lineStr = log[i].ToString();
        if (FilterTextLine(lineStr))
          outText.append(lineStr, "\n");
      }
    }

    if (outText.back() == '\n')
      outText.length--;

    CallMethod("setlogtext", (epString)outText);
  }
}

void UIConsole::OnLogChanged()
{
  epSlice<LogLine> log = pKernel->GetLogger()->GetLog();
  LogLine &line = log.back();

  // Do filtering
  if (!logFilter.FilterLogLine(line))
    return;

  epMutableString<256> lineStr = line.ToString(LogDefaults::Format);

  if(!FilterTextLine(lineStr))
    return;

  if (bOutputsMerged)
    CallMethod("appendconsoletext", (epString)lineStr);
  else
    CallMethod("appendlogtext", (epString)lineStr);
}

void UIConsole::RelayInput(epString str)
{
  auto spInBuffer = pKernel->CreateComponent<Buffer>();
  spInBuffer->Allocate(1024);
  spInBuffer->CopyBuffer(str);
  spInStream->SetBuffer(spInBuffer);
}

void UIConsole::OnStreamOutput()
{
  int64_t newPos = spOutStream->GetPos();
  epMutableString<0> buf;
  buf.reserve(newPos - pos);

  spOutStream->Seek(SeekOrigin::Begin, 0);
  epSlice<void> readSlice = spOutStream->Read(buf.getBuffer());
  epString readStr = (epString &)readSlice;
  spOutStream->Seek(SeekOrigin::Begin, 0);

  while (!readStr.empty())
  {
    epString token = readStr.popToken("\n");
    if (!token.empty())
    {
      consoleLines.pushBack(ConsoleLine(token));
      if (FilterTextLine(token))
        CallMethod("appendconsoletext", token);
    }
  }
}

UIConsole::ConsoleLine::ConsoleLine(epString text)
{
  this->text = text;
  ordering = epPerformanceCounter();
}

// Filter getter/setter helper functions

epString UIConsole::GetFilterComponents() const
{
  epMutableString<1024> str;

  epArray<epSharedString> comps = logFilter.GetComponents();
  for (epSharedString &c: comps)
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

void UIConsole::SetFilterComponents(epString str)
{
  epString token;
  epArray<epString> comps;

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

bool UIConsole::FilterTextLine(epString line) const
{
  if (!textFilter.empty() && line.findFirstIC(textFilter) == line.length)
    return false;

  return true;
}

}
