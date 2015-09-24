
#include "helpers.h"

epSharedSlice<const epKeyValuePair> udParseCommandLine(const char *pCommandLine)
{
  epArray<const epKeyValuePair, 64> output;
  output.concat(epKeyValuePair(nullptr, nullptr)); // TODO: populate argv[0] with the exe path

  // TODO: more comprehensive version that parses for '=' to distinguish key=value ??

  size_t i = 0, start = 0;
  bool bInQuotes = false;
  while (pCommandLine[i] != 0)
  {
    while (isWhitespace(pCommandLine[i]))
      ++i;
    start = i;
    while (pCommandLine[i] && (!isWhitespace(pCommandLine[i]) || bInQuotes))
    {
      if (pCommandLine[i] == '\"')
        bInQuotes = !bInQuotes;
      ++i;
    }
    if (i > start)
      output.pushBack(epKeyValuePair(nullptr, epString(pCommandLine + start, i - start)));
  }

  return epSharedSlice<const epKeyValuePair>(output);
}

epSharedSlice<const epKeyValuePair> udParseCommandLine(int argc, char *argv[])
{
  epArray<const epKeyValuePair, 64> output;
  output.reserve(argc);

  // TODO: more comprehensive version that parses for '=' to distinguish key=value ??

  for (int i = 0; i < argc; ++i)
    output.pushBack(epKeyValuePair(nullptr, epString(argv[i])));

  return epSharedSlice<const epKeyValuePair>(output.slice(0, argc));
}

epSharedSlice<const epKeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[])
{
  epArray<const epKeyValuePair, 64> output;
  output.reserve(argc);

  for (uint32_t i = 0; i < argc; ++i)
    output.pushBack(epKeyValuePair(epString(argn[i]), epString(argv[i])));

  return epSharedSlice<const epKeyValuePair>(output.slice(0, argc));
}
