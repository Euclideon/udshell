#pragma once
#ifndef EPHELPERS_H
#define EPHELPERS_H

#include "ep/cpp/variant.h"

Array<const KeyValuePair> epParseCommandLine(const char *pCommandLine);
Array<const KeyValuePair> epParseCommandLine(int argc, char *argv[]);
Array<const KeyValuePair> epParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

int epGetHardwareThreadCount();

#endif // EPHELPERS_H
