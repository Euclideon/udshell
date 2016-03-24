#pragma once
#ifndef EPHELPERS_H
#define EPHELPERS_H

#include "ep/cpp/variant.h"

ep::Array<const ep::KeyValuePair> epParseCommandLine(const char *pCommandLine);
ep::Array<const ep::KeyValuePair> epParseCommandLine(int argc, char *argv[]);
ep::Array<const ep::KeyValuePair> epParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

int epGetHardwareThreadCount();

#endif // EPHELPERS_H
