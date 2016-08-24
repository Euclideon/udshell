#include "libep_internal.h"
#include "ep/version.h"
#include "ep/cpp/platform.h"

#include "ep/cpp/string.h"

epVersion epVersionExtract(const char *version, const char *delimiters)
{
  ep::String verStr(version);
  epVersion v;
  v.major = static_cast<int>(verStr.popToken(delimiters).parseInt());
  v.minor = static_cast<int>(verStr.popToken(delimiters).parseInt());
  v.revision = static_cast<int>(verStr.popToken(delimiters).parseInt());
  return v;
}

bool epVersionIsCompatible(const char *v1, const char *v2, const char *delimiters)
{
  epVersion version1 = epVersionExtract(v1, delimiters);
  epVersion version2 = epVersionExtract(v2, delimiters);

  // Versions are considered API compatible if they have matching major numbers
  if (version1.major != version2.major)
    return false;

  // Pre-Release versions (major == 0) are considered less stable and require matching minor numbers
  return (version1.major != 0 || (version1.minor == version2.minor));
}
