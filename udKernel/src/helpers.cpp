
#include "helpers.h"

namespace ud
{

udRCSlice<KeyValuePair> udParseCommandLine(const char *pCommandLine)
{
  udFixedSlice<KeyValuePair, 64> output;
  output.concat(KeyValuePair(nullptr, nullptr)); // TODO: populate argv[0] with the exe path

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
      output.pushBack(KeyValuePair(nullptr, udString(pCommandLine + start, i - start)));
  }

  return udRCSlice<KeyValuePair>(output);
}

udRCSlice<KeyValuePair> udParseCommandLine(int argc, char *argv[])
{
  udFixedSlice<KeyValuePair, 64> output;
  output.reserve(argc);

  // TODO: more comprehensive version that parses for '=' to distinguish key=value ??

  for (int i = 0; i < argc; ++i)
    output.pushBack(KeyValuePair(nullptr, udString(argv[i])));

  return udRCSlice<KeyValuePair>(output.slice(0, argc));
}

udRCSlice<KeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[])
{
  udFixedSlice<KeyValuePair, 64> output;
  output.reserve(argc);

  for (uint32_t i = 0; i < argc; ++i)
    output.pushBack(KeyValuePair(udString(argn[i]), udString(argv[i])));

  return udRCSlice<KeyValuePair>(output.slice(0, argc));
}

} // namespace ud
