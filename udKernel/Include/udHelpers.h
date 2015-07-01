#pragma once
#ifndef UDHELPERS_H
#define UDHELPERS_H

#include "udString.h"


struct udKeyValuePair
{
  udKeyValuePair(udString key, udString value) : key(key), value(value) {}

  udString key, value;
};
typedef udSlice<udKeyValuePair> udInitParams;


udRCSlice<udKeyValuePair> udParseCommandLine(const char *pCommandLine);
udRCSlice<udKeyValuePair> udParseCommandLine(int argc, char *argv[]);
udRCSlice<udKeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

#endif // UDHELPERS_H
