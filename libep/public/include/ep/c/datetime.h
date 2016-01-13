#if !defined(_EPDATETIME_H)
#define _EPDATETIME_H

#include "ep/c/platform.h"

#include <time.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define DEFAULT_DATETIME_STRING "%Y-%m-%d %H:%M:%S"
#define START_YEAR -10000 // 10000 BC

struct epDateTime
{
  uint64_t isdst : 1;
  uint64_t wday : 3;
  uint64_t mday : 5;
  uint64_t mon : 4;
  uint64_t msec : 10;
  uint64_t sec : 6;
  uint64_t min : 6;
  uint64_t hour : 5;
  uint64_t yday : 9;
  uint64_t year : 15;
};

#if defined(__cplusplus)
}
#endif

#endif // _EPDATETIME_H
