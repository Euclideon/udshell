#include "ep/cpp/platform.h"
#include "helpers.h"
#if defined(EP_LINUX)
# include <unistd.h>
#endif

namespace ep {

Array<const KeyValuePair> parseCommandLine(const char *pCommandLine)
{
  Array<const KeyValuePair> output(ep::Concat, KeyValuePair(nullptr, nullptr)); // TODO: populate argv[0] with the exe path

  // TODO: more comprehensive version that parses for '=' to distinguish key=value ??

  size_t i = 0, start = 0, numArgs = 0;
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
      output.pushBack(KeyValuePair(numArgs++, String(pCommandLine + start, i - start)));
  }

  return output;
}

Array<const KeyValuePair> parseCommandLine(int argc, char *argv[])
{
  Array<const KeyValuePair> output(ep::Reserve, argc);

  for (int i = 0; i < argc; ++i)
    output.pushBack(KeyValuePair(i, argv[i]));

  // parse '=' arguments
  for (int i = 1; i < argc; ++i)
  {
    String arg = output[i].value.asString();
    size_t eq = arg.findFirst('=');
    if (eq < arg.length)
    {
      String val = arg.slice(eq+1, arg.length).trim();
      if(val.length >= 2)
      {
        if ((val[0] == '\'' && val[val.length-1] == '\'') || (val[0] == '\"' && val[val.length-1] == '\"'))
          val = val.slice(1, val.length-1);
      }
      output.pushBack(KeyValuePair(arg.slice(0, eq).trim(), val));
    }
  }

  return output;
}

Array<const KeyValuePair> parseCommandLine(uint32_t argc, const char* argn[], const char* argv[])
{
  Array<const KeyValuePair> output(ep::Reserve, argc);

  for (uint32_t i = 0; i < argc; ++i)
    output.pushBack(KeyValuePair(String(argn[i]), String(argv[i])));

  return output;
}

int getHardwareThreadCount()
{
#if defined(EP_WINDOWS)
  DWORD_PTR processMask;
  DWORD_PTR systemMask;

  if (::GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask))
  {
    int hardwareThreadCount = 0;
    while (processMask)
    {
      ++hardwareThreadCount;
      processMask &= processMask - 1; // Clear LSB
    }
    return hardwareThreadCount;
  }
#elif defined(EP_LINUX)
  return sysconf(_SC_NPROCESSORS_ONLN);
#endif

  return 1;
}

} // namespace ep
