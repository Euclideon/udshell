#include "components/console.h"
#include "ep/cpp/component/memstream.h"
#include "ep/cpp/component/broadcaster.h"
#include "components/logger.h"
#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/kernel.h"
#include "hal/timer.h"

namespace ep {

Array<const PropertyInfo> Console::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(FilterComponents, "List of Components to filter the log text by", nullptr, 0),
    EP_MAKE_PROPERTY(FilterText, "Text string to filter console and log lines by", nullptr, 0),
    EP_MAKE_PROPERTY_RO(NumConsoleLines, "Number of console lines to output", nullptr, 0),
    EP_MAKE_PROPERTY_RO(HistoryLength, "Number of lines in the input history", nullptr, 0),
    EP_MAKE_PROPERTY_RO(Title, "The console page's title", nullptr, 0),
    EP_MAKE_PROPERTY_EXPLICIT("HasInput", "Bool specifies whether this console accepts input", EP_MAKE_GETTER(HasInput), nullptr, nullptr, 0),
    EP_MAKE_PROPERTY_EXPLICIT("OutputLog", "Bool specifies whether this console outputs the application log", EP_MAKE_GETTER(OutputLog), nullptr, nullptr, 0),
  };
}
Array<const MethodInfo> Console::GetMethods() const
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
    EP_MAKE_METHOD(GetHistoryLine, "Get the line at specified index from the input history. Negative numbers count from end."),
    EP_MAKE_METHOD(AddBroadcaster, "Output the given broadcaster's text to the console"),
    EP_MAKE_METHOD(RemoveBroadcaster, "Stop outputting the given broadcaster's text to the console"),
  };
}

Console::Console(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  const Variant *vTitle = initParams.get("title");
  if (!vTitle || !vTitle->is(Variant::Type::String))
    EPTHROW_ERROR(Result::InvalidArgument, "Missing or invalid 'title'");
  title = vTitle->asString();

  const Variant *vSetOutputFunc = initParams.get("setOutputFunc");
  if (!vSetOutputFunc || !vSetOutputFunc->is(Variant::SharedPtrType::Delegate))
    EPTHROW_ERROR(Result::InvalidArgument, "Missing or invalid 'setOutputFunc'");
  setOutputFunc = vSetOutputFunc->as<Delegate<void(String)>>();

  const Variant *vAppendOutputFunc = initParams.get("appendOutputFunc");
  if (!vAppendOutputFunc || !vAppendOutputFunc->is(Variant::SharedPtrType::Delegate))
    EPTHROW_ERROR(Result::InvalidArgument, "Missing or invalid 'appendOutputFunc'");
  appendOutputFunc = vAppendOutputFunc->as<Delegate<void(String)>>();

  const Variant *vBHasInput = initParams.get("hasInput");
  if (!vBHasInput || !vBHasInput->is(Variant::Type::Bool))
    EPTHROW_ERROR(Result::InvalidArgument, "Missing or invalid 'hasInput'");
  bHasInput = vBHasInput->asBool();

  if (bHasInput)
  {
    const Variant *vInputFunc = initParams.get("inputFunc");
    if (!vInputFunc || !vInputFunc->is(Variant::SharedPtrType::Delegate))
      EPTHROW_ERROR(Result::InvalidArgument, "Missing or invalid 'inputFunc'");
    inputFunc = vInputFunc->as<Delegate<void(String)>>();

    const Variant *vHistoryFileName = initParams.get("historyFileName");
    if (!vHistoryFileName || !vHistoryFileName->is(Variant::Type::String))
      EPTHROW_ERROR(Result::InvalidArgument, "Missing or invalid 'historyFileName'");
    historyFileName = vHistoryFileName->asString();

    // Console input history file
    epscope(fail) { if (!spHistoryFile) LogError("Console -- Could not open history file \"{0}\"", historyFileName); };
    spHistoryFile = pKernel->CreateComponent<File>({ { "path", historyFileName },{ "flags", FileOpenFlags::Append | FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Text } });

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

  const Variant *vBOutputLog = initParams.get("outputLog");
  if (!vBOutputLog || !vBOutputLog->is(Variant::Type::Bool))
    EPTHROW_ERROR(Result::InvalidArgument, "Missing or invalid 'bOutputLog'");
  bOutputLog = vBOutputLog->asBool();

  spLogger = pKernel->GetLogger();
  if (bOutputLog)
    pKernel->GetLogger()->Changed.Subscribe(this, &Console::OnLogChanged);
}

Console::~Console()
{
  for (BroadcasterRef spBC : outputBCArray)
    spBC->Written.Unsubscribe(this, &Console::OnConsoleOutput);

  if(bOutputLog)
    spLogger->Changed.Unsubscribe(this, &Console::OnLogChanged);
}

void Console::AddBroadcaster(BroadcasterRef spBC)
{
  outputBCArray.pushBack(spBC);
  spBC->Written.Subscribe(this, &Console::OnConsoleOutput);
}

void Console::RemoveBroadcaster(BroadcasterRef spBC)
{
  outputBCArray.removeFirstSwapLast(spBC);
}

void Console::RebuildOutput()
{
  filteredConsole = nullptr;

  for (size_t i = 0; i < consoleLines.length; i++)
  {
    if (FilterTextLine(consoleLines[i].text)
        && (consoleLines[i].logIndex == -1 || logFilter.FilterLogLine(*spLogger->GetLogLine(consoleLines[i].logIndex))))
      filteredConsole.pushBack((int)i);
  }

  // TODO Change below when adding virtual scrollbar support

  MutableString<0> outText(Reserve, 256 * filteredConsole.length);

  for (int i : filteredConsole)
    outText.append(consoleLines[i].text, "\n");

  if (!outText.empty() && outText.back() == '\n')
    outText.length--;

  setOutputFunc((String)outText);
}

void Console::OnLogChanged()
{
  Slice<LogLine> log = pKernel->GetLogger()->GetLog();
  LogLine &line = log.back();

  consoleLines.pushBack(ConsoleLine(line.ToString(), (int)log.length - 1, spLogger->GetLogLine((int)log.length - 1)->ordering));
  ConsoleLine &cLine = consoleLines.back();

  // Do filtering
  if (!logFilter.FilterLogLine(line) || !FilterTextLine(cLine.text))
    return;

  filteredConsole.pushBack((int)consoleLines.length - 1);
  appendOutputFunc(cLine.text);
}

void Console::OnConsoleOutput(Slice<const void> buf)
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

        appendOutputFunc(token);
      }
    }
  }
}

void Console::RelayInput(String str)
{
  AppendHistory(str);
  pKernel->GetLogger()->Changed.Subscribe(this, &Console::OnLogChanged);
  inputFunc(str);
  pKernel->GetLogger()->Changed.Unsubscribe(this, &Console::OnLogChanged);
}

Console::ConsoleLine::ConsoleLine(String text, int logIndex, double ordering)
{
  this->text = text;
  this->logIndex = logIndex;
  if (ordering == 0.0)
    ordering = epPerformanceCounter();
}

// Filter getter/setter helper functions

String Console::GetFilterComponents() const
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

void Console::SetFilterComponents(String str)
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

bool Console::FilterTextLine(String line) const
{
  if (!textFilter.empty() && line.findFirstIC(textFilter) == line.length)
    return false;

  return true;
}

// History functions

void Console::AppendHistory(String str)
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
