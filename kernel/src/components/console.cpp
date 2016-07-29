#include "components/console.h"
#include "ep/cpp/component/memstream.h"
#include "ep/cpp/component/broadcaster.h"
#include "components/logger.h"
#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/kernel.h"
#include "hal/timer.h"

namespace ep {

Array<const PropertyInfo> Console::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("filterComponents", getFilterComponents, setFilterComponents, "List of Components to filter the log text by", nullptr, 0),
    EP_MAKE_PROPERTY("filterText", getFilterText, setFilterText, "Text string to filter console and log lines by", nullptr, 0),
    EP_MAKE_PROPERTY_RO("numConsoleLines", getNumConsoleLines, "Number of console lines to output", nullptr, 0),
    EP_MAKE_PROPERTY_RO("historyLength", getHistoryLength, "Number of lines in the input history", nullptr, 0),
    EP_MAKE_PROPERTY_RO("title", getTitle, "The console page's title", nullptr, 0),
    EP_MAKE_PROPERTY_RO("hasInput", hasInput, "Bool specifies whether this console accepts input", nullptr, 0),
    EP_MAKE_PROPERTY_RO("outputLog", outputLog, "Bool specifies whether this console outputs the application log", nullptr, 0),
  };
}
Array<const MethodInfo> Console::getMethods() const
{
  return{
    EP_MAKE_METHOD(getFilterLevel, "Get the filter level for the given log category"),
    EP_MAKE_METHOD(setFilterLevel, "Set the filter level for the given log category"),
    EP_MAKE_METHOD(isFilterCategoryEnabled, "Returns whether the given log category is enabled in the filter"),
    EP_MAKE_METHOD(enableFilterCategory, "Enables the given log categories in the log filter"),
    EP_MAKE_METHOD(disableFilterCategory, "Disables the given log categories in the log filter"),
    EP_MAKE_METHOD(rebuildOutput, "Rebuild output text and send to UI"),
    EP_MAKE_METHOD(relayInput, "Send input to the Kernel's input stream"),
    EP_MAKE_METHOD(appendHistory, "Add a line to the end of the input history"),
    EP_MAKE_METHOD(getHistoryLine, "Get the line at specified index from the input history. Negative numbers count from end."),
    EP_MAKE_METHOD(addBroadcaster, "Output the given broadcaster's text to the console"),
    EP_MAKE_METHOD(removeBroadcaster, "Stop outputting the given broadcaster's text to the console"),
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
    epscope(fail) { if (!spHistoryFile) logError("Console -- Could not open history file \"{0}\"", historyFileName); };
    spHistoryFile = pKernel->createComponent<File>({ { "path", historyFileName },{ "flags", FileOpenFlags::Append | FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Text } });

    size_t len = (size_t)spHistoryFile->length();
    Array<char> buffer(Reserve, len);
    buffer.length = spHistoryFile->read(buffer.getBuffer()).length;

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

  spLogger = pKernel->getLogger();
  if (bOutputLog)
    pKernel->getLogger()->changed.subscribe(this, &Console::onLogChanged);
}

Console::~Console()
{
  for (BroadcasterRef spBC : outputBCArray)
    spBC->written.unsubscribe(this, &Console::onConsoleOutput);

  if(bOutputLog)
    spLogger->changed.unsubscribe(this, &Console::onLogChanged);
}

void Console::addBroadcaster(BroadcasterRef spBC)
{
  outputBCArray.pushBack(spBC);
  spBC->written.subscribe(this, &Console::onConsoleOutput);
}

void Console::removeBroadcaster(BroadcasterRef spBC)
{
  outputBCArray.removeFirstSwapLast(spBC);
}

void Console::rebuildOutput()
{
  filteredConsole = nullptr;

  for (size_t i = 0; i < consoleLines.length; i++)
  {
    if (filterTextLine(consoleLines[i].text)
        && (consoleLines[i].logIndex == -1 || logFilter.filterLogLine(*spLogger->getLogLine(consoleLines[i].logIndex))))
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

void Console::onLogChanged()
{
  Slice<LogLine> log = pKernel->getLogger()->getLog();
  LogLine &line = log.back();

  consoleLines.pushBack(ConsoleLine(line.toString(), (int)log.length - 1, spLogger->getLogLine((int)log.length - 1)->ordering));
  ConsoleLine &cLine = consoleLines.back();

  // Do filtering
  if (!logFilter.filterLogLine(line) || !filterTextLine(cLine.text))
    return;

  filteredConsole.pushBack((int)consoleLines.length - 1);
  appendOutputFunc(cLine.text);
}

void Console::onConsoleOutput(Slice<const void> buf)
{
  String readStr = (String &)buf;

  while (!readStr.empty())
  {
    String token = readStr.popToken("\n");
    if (!token.empty())
    {
      consoleLines.pushBack(ConsoleLine(token));
      if (filterTextLine(token))
      {
        filteredConsole.pushBack((int)consoleLines.length - 1);

        appendOutputFunc(token);
      }
    }
  }
}

void Console::relayInput(String str)
{
  appendHistory(str);
  pKernel->getLogger()->changed.subscribe(this, &Console::onLogChanged);
  inputFunc(str);
  pKernel->getLogger()->changed.unsubscribe(this, &Console::onLogChanged);
}

Console::ConsoleLine::ConsoleLine(String text, int logIndex, double ordering)
{
  this->text = text;
  this->logIndex = logIndex;
  if (ordering == 0.0)
    ordering = epPerformanceCounter();
}

// Filter getter/setter helper functions

String Console::getFilterComponents() const
{
  MutableString<1024> str;

  Array<SharedString> comps = logFilter.getComponents();
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

void Console::setFilterComponents(String str)
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

  logFilter.setComponents(comps);
  rebuildOutput();
}

bool Console::filterTextLine(String line) const
{
  if (!textFilter.empty() && line.findFirstIC(textFilter) == line.length)
    return false;

  return true;
}

// History functions

void Console::appendHistory(String str)
{
  history.concat(str);

  while (!str.empty())
  {
    String token = str.popToken("\n");
    if (!token.empty())
    {
      if (str.empty())
        spHistoryFile->writeLn(token);
      else
      {
        spHistoryFile->write(token);
        spHistoryFile->write(String(" \\\n"));
      }
    }
  }
}

}
