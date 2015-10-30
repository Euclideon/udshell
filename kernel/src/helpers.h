#pragma once
#ifndef EPHELPERS_H
#define EPHELPERS_H

#include "ep/cpp/variant.h"

Array<const KeyValuePair> udParseCommandLine(const char *pCommandLine);
Array<const KeyValuePair> udParseCommandLine(int argc, char *argv[]);
Array<const KeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

#endif // EPHELPERS_H
