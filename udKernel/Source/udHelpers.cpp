
#include "udHelpers.h"

udRCSlice<udKeyValuePair> udParseCommandLine(const char *pCommandLine)
{
  udFixedSlice<udKeyValuePair, 64> output;
  output.concat(udKeyValuePair(nullptr, nullptr)); // TODO: populate argv[0] with the exe path

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
      output.pushBack(udKeyValuePair(nullptr, udString(pCommandLine + start, i - start)));
  }

  return udRCSlice<udKeyValuePair>(output);
}

udRCSlice<udKeyValuePair> udParseCommandLine(int argc, char *argv[])
{
  udFixedSlice<udKeyValuePair, 64> output;
  output.reserve(argc);

  // TODO: more comprehensive version that parses for '=' to distinguish key=value ??

  for (int i = 0; i < argc; ++i)
    output.pushBack(udKeyValuePair(nullptr, argv[i]));

  return udRCSlice<udKeyValuePair>(output.slice(0, argc));
}

udRCSlice<udKeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[])
{
  udFixedSlice<udKeyValuePair, 64> output;
  output.reserve(argc);

  for (uint32_t i = 0; i < argc; ++i)
    output.pushBack(udKeyValuePair(argn[i], argv[i]));

  return udRCSlice<udKeyValuePair>(output.slice(0, argc));
}
