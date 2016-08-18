#pragma once
#ifndef EP_HALLIBRARY_H
#define EP_HALLIBRARY_H

#include "ep/cpp/platform.h"

typedef void *epLibrary;

bool epLibrary_Open(epLibrary *pLibrary, const char *filename);
bool epLibrary_Close(epLibrary library);
void *epLibrary_GetFunction(epLibrary library, const char *funcName);
char *epLibrary_GetLastError();

#endif // EP_HALLIBRARY_H
