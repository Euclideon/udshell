#pragma once
#ifndef EPHELPERS_H
#define EPHELPERS_H

#include "ep/cpp/variant.h"

namespace ep {

ep::Array<const ep::KeyValuePair> parseCommandLine(const char *pCommandLine);
ep::Array<const ep::KeyValuePair> parseCommandLine(int argc, char *argv[]);
ep::Array<const ep::KeyValuePair> parseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

int getHardwareThreadCount();

} // namespace ep

#endif // EPHELPERS_H
