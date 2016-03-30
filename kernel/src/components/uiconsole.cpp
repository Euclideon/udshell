#include "components/uiconsole.h"
#include "components/memstream.h"
#include "ep/cpp/component/broadcaster.h"
#include "components/logger.h"
#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/component/commandmanager.h"
#include "components/file.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/kernel.h"
#include "components/lua.h"
#include "hal/haltimer.h"

namespace ep {

Array<const PropertyInfo> UIConsole::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(FilterComponents, "List of Components to filter the log text by", nullptr, 0),
    EP_MAKE_PROPERTY(FilterText, "Text string to filter console and log lines by", nullptr, 0),
    EP_MAKE_PROPERTY_RO(NumConsoleLines, "Number of console lines to output", nullptr, 0),
    EP_MAKE_PROPERTY_RO(NumLogLines, "Number of log lines to output", nullptr, 0),
    EP_MAKE_PROPERTY_RO(HistoryLength, "Number of lines in the input history", nullptr, 0)
  };
}
Array<const MethodInfo> UIConsole::GetMethods() const
{
  return{
    EP_MAKE_METHOD(GetFilterLevel, "Get the filter level for the given log category"),
    EP_MAKE_METHOD(SetFilterLevel, "Set the filter level for the given log category"),
    EP_MAKE_METHOD(IsFilterCategoryEnabled, "Returns whether the given log category is enabled in the filter"),
    EP_MAKE_METHOD(EnableFilterCategory, "Enables the given log categories in the log filter"),
    EP_MAKE_METHOD(DisableFilterCategory, "Disables the given log categories in the log filter"),
    EP_MAKE_METHOD(RebuildOutput, "Rebuild output text and send to UI"),
    EP_MAKE_METHOD(RelayInput, "Send input to the Kernel's input stream"),
    EP_MAKE_METHOD(AppendHistory, "Add a line to the end of the input history"),
    EP_MAKE_METHOD(GetHistoryLine, "Get the line at specified index from the input history. Negative numbers count from end.")
  };
}

UIConsole::UIConsole(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : UIComponent(pType, pKernel, uid, initParams)
{
  spConsoleOut = pKernel->GetStdOutBroadcaster();
  spConsoleOut->Written.Subscribe(this, &UIConsole::OnConsoleOutput);
  spConsoleErr = pKernel->GetStdErrBroadcaster();
  spConsoleErr->Written.Subscribe(this, &UIConsole::OnConsoleOutput);

  auto spLua = pKernel->GetLua();
  spLuaOut = spLua->GetOutputBroadcaster();
  spLuaOut->Written.Subscribe(this, &UIConsole::OnConsoleOutput);

  spLogger = pKernel->GetLogger();
  pKernel->GetLogger()->Changed.Subscribe(this, &UIConsole::OnLogChanged);

  auto spCommandManager = pKernel->GetCommandManager();
  spCommandManager->RegisterCommand("showhideconsolewindow", Delegate<void(Variant::VarMap)>(this, &UIConsole::ToggleVisible), "", "", "`");

  // Console input history file
  epscope(fail) { if(!spHistoryFile) LogError("Console -- Could not open history file \"{0}\"", historyFileName); };
  spHistoryFile = pKernel->CreateComponent<File>({ { "path", historyFileName }, { "flags", FileOpenFlags::Append | FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Text } });

  size_t len = (size_t)spHistoryFile->Length();
  Array<char> buffer(Reserve, len);
  buffer.length = spHistoryFile->Read(buffer.getBuffer()).length;

  String str = buffer;
  MutableString<1024> multiLineString;
  while (!str.empty())
  {
    String token = str.popToken<true>("\n");
    token = token.trim<false, true>();
    if (!token.empty())
    {
      if (token.endsWith(" \\") || token.endsWith("\t\\"))
      {
        token.length -= 2;
        multiLineString.append(token, "\n");
      }
      else if (!multiLineString.empty())
      {
        multiLineString.append(token);
        history.concat(multiLineString);
        multiLineString = "";
      }
      else
        history.concat(token);
    }
  }
  if (!multiLineString.empty())
  {
    multiLineString.length--;
    history.concat(multiLineString);
  }
}

UIConsole::~UIConsole()
{
  spConsoleOut->Written.Unsubscribe(this, &UIConsole::OnConsoleOutput);
  spConsoleErr->Written.Unsubscribe(this, &UIConsole::OnConsoleOutput);
  spLuaOut->Written.Unsubscribe(this, &UIConsole::OnConsoleOutput);
  spLogger->Changed.Unsubscribe(this, &UIConsole::OnLogChanged);
}

void UIConsole::ToggleVisible(Variant::VarMap params)
{
  Call("togglevisible", nullptr);
}

void UIConsole::RebuildOutput()
{
  filteredConsole = nullptr;
  filteredLog = nullptr;

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

  Call("setconsoletext", (String)outText);

  outText.length = 0;

  for (int i : filteredLog)
    outText.append(logLines[i].text, "\n");

  if (!outText.empty() && outText.back() == '\n')
    outText.length--;

  Call("setlogtext", (String)outText);
}

void UIConsole::OnLogChanged()
{
  Slice<LogLine> log = pKernel->GetLogger()->GetLog();
  LogLine &line = log.back();

  logLines.pushBack(ConsoleLine(line.ToString(), (int)log.length - 1, spLogger->GetLogLine((int)log.length - 1)->ordering));
  ConsoleLine &cLine = logLines.back();

  if (bOutputLogToConsole)
    OnConsoleOutput(line.text);

  // Do filtering
  if (!logFilter.FilterLogLine(line) || !FilterTextLine(cLine.text))
    return;

  filteredLog.pushBack((int)logLines.length - 1);
  Call("appendlogtext", cLine.text);
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
        filteredConsole.pushBack((int)consoleLines.length - 1);

        Call("appendconsoletext", token);
      }
    }
  }
}

void UIConsole::RelayInput(String str)
{
  AppendHistory(str);
  bOutputLogToConsole = true;
  pKernel->Exec(str);
  bOutputLogToConsole = false;
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

// History functions

void UIConsole::AppendHistory(String str)
{
  history.concat(str);

  while (!str.empty())
  {
    String token = str.popToken("\n");
    if (!token.empty())
    {
      if (str.empty())
        spHistoryFile->WriteLn(token);
      else
      {
        spHistoryFile->Write(token);
        spHistoryFile->Write(String(" \\\n"));
      }
    }
  }
}

}
