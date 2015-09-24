#pragma once
#ifndef EPHELPERS_H
#define EPHELPERS_H

#include "ep/epvariant.h"

epSharedSlice<const epKeyValuePair> udParseCommandLine(const char *pCommandLine);
epSharedSlice<const epKeyValuePair> udParseCommandLine(int argc, char *argv[]);
epSharedSlice<const epKeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

#endif // EPHELPERS_H
