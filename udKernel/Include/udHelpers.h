#pragma once
#ifndef UDHELPERS_H
#define UDHELPERS_H

#include "udVariant.h"

namespace udKernel
{
  udRCSlice<KeyValuePair> udParseCommandLine(const char *pCommandLine);
  udRCSlice<KeyValuePair> udParseCommandLine(int argc, char *argv[]);
  udRCSlice<KeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);
} // namespace udKernel

#endif // UDHELPERS_H
