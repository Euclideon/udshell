#pragma once
#if !defined(EPVERSION_H)
#define EPVERSION_H

#define EP_APIVERSION "0.10.0"
#define EPKERNEL_PLUGINVERSION EP_APIVERSION

struct epVersion
{
  int major;
  int minor;
  int revision;
};

epVersion epVersionExtract(const char *version, const char *delimiters = ".");
bool epVersionIsCompatible(const char *v1, const char *v2, const char *delimiters = ".");

#endif // EPVERSION_H
