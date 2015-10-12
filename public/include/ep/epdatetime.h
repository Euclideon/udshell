#pragma once
#if !defined(_EPDATETIME_H)
#define _EPDATETIME_H

#include <time.h>
#include "ep/epvariant.h"

#define DEFAULT_DATETIME_STRING "%d/%m/%y %H:%M:%S"
#define START_YEAR -10000 // 10000 BC

class epDateTime
{
public:
  epDateTime();
  epDateTime(time_t ti);

  epMutableString<64> ToString(epString format = nullptr) const;
  time_t ToTimeStamp() const;
  int16_t GetAbsoluteYear() const;

  bool operator< (epDateTime other) const { return (uint64_t &)*this < (uint64_t &)other; }
  bool operator> (epDateTime other) const { return (uint64_t &)*this > (uint64_t &)other; }
  bool operator<=(epDateTime other) const { return (uint64_t &)*this <= (uint64_t &)other; }
  bool operator>=(epDateTime other) const { return (uint64_t &)*this >= (uint64_t &)other; }
  bool operator==(epDateTime other) const { return (uint64_t &)*this == (uint64_t &)other; }
  bool operator!=(epDateTime other) const { return (uint64_t &)*this != (uint64_t &)other; }

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

protected:
  void TimeStampToDateTime(time_t ti);
  void TmToDateTime(tm *pTm);
  tm DateTimeToTm() const;

  friend epVariant epToVariant(const epDateTime& dt);
  friend void epFromVariant(const epVariant &variant, epDateTime *pDt);
};

ptrdiff_t epStringify(epSlice<char> buffer, epString format, const epDateTime &dt, const epVarArg *epUnusedParam(pArgs));

epVariant epToVariant(const epDateTime& dt);
void epFromVariant(const epVariant &variant, epDateTime *pDt);

#endif // _EPDATETIME_H
