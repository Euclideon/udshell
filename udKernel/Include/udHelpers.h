#pragma once
#ifndef UDHELPERS_H
#define UDHELPERS_H

#include "udVariant.h"

udRCSlice<udKeyValuePair> udParseCommandLine(const char *pCommandLine);
udRCSlice<udKeyValuePair> udParseCommandLine(int argc, char *argv[]);
udRCSlice<udKeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

#endif // UDHELPERS_H
