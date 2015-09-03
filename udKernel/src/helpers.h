#pragma once
#ifndef UDHELPERS_H
#define UDHELPERS_H

#include "util/udvariant.h"

udSharedSlice<const udKeyValuePair> udParseCommandLine(const char *pCommandLine);
udSharedSlice<const udKeyValuePair> udParseCommandLine(int argc, char *argv[]);
udSharedSlice<const udKeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

#endif // UDHELPERS_H
