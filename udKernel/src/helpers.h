#pragma once
#ifndef UDHELPERS_H
#define UDHELPERS_H

#include "util/udvariant.h"

udRCSlice<const udKeyValuePair> udParseCommandLine(const char *pCommandLine);
udRCSlice<const udKeyValuePair> udParseCommandLine(int argc, char *argv[]);
udRCSlice<const udKeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

#endif // UDHELPERS_H
