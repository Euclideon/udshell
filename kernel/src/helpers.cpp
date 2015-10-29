
#include "helpers.h"

Array<const KeyValuePair> udParseCommandLine(const char *pCommandLine)
{
  Array<const KeyValuePair> output;
  output.concat(KeyValuePair(nullptr, nullptr)); // TODO: populate argv[0] with the exe path

  // TODO: more comprehensive version that parses for '=' to distinguish key=value ??

  size_t i = 0, start = 0;
  bool bInQuotes = false;
  while (pCommandLine[i] != 0)
  {
    while (epIsWhitespace(pCommandLine[i]))
      ++i;
    start = i;
    while (pCommandLine[i] && (!epIsWhitespace(pCommandLine[i]) || bInQuotes))
    {
      if (pCommandLine[i] == '\"')
        bInQuotes = !bInQuotes;
      ++i;
    }
    if (i > start)
      output.pushBack(KeyValuePair(nullptr, String(pCommandLine + start, i - start)));
  }

  return std::move(output);
}

Array<const KeyValuePair> udParseCommandLine(int argc, char *argv[])
{
  Array<const KeyValuePair> output;
  output.reserve(argc);

  // TODO: more comprehensive version that parses for '=' to distinguish key=value ??

  for (int i = 0; i < argc; ++i)
    output.pushBack(KeyValuePair(nullptr, String(argv[i])));

  return std::move(output);
}

Array<const KeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[])
{
  Array<const KeyValuePair> output;
  output.reserve(argc);

  for (uint32_t i = 0; i < argc; ++i)
    output.pushBack(KeyValuePair(String(argn[i]), String(argv[i])));

  return std::move(output);
}
