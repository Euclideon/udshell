#pragma once
#ifndef EPHELPERS_H
#define EPHELPERS_H

#include "ep/cpp/variant.h"

SharedSlice<const KeyValuePair> udParseCommandLine(const char *pCommandLine);
SharedSlice<const KeyValuePair> udParseCommandLine(int argc, char *argv[]);
SharedSlice<const KeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

#endif // EPHELPERS_H
