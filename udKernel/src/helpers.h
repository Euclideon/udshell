#pragma once
#ifndef UDHELPERS_H
#define UDHELPERS_H

#include "variant.h"

namespace ud
{
  udRCSlice<KeyValuePair> udParseCommandLine(const char *pCommandLine);
  udRCSlice<KeyValuePair> udParseCommandLine(int argc, char *argv[]);
  udRCSlice<KeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);
} // namespace ud

#endif // UDHELPERS_H
