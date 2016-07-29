#pragma once
#if !defined(_EPDATETIME_HPP)
#define _EPDATETIME_HPP

#define DEFAULT_DATETIME_STRING "%Y-%m-%d %H:%M:%S"
#define START_YEAR -10000 // 10000 BC

#include "ep/cpp/variant.h"

namespace ep {

class DateTime
{
public:
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

  DateTime();
  DateTime(time_t ti);

  MutableString64 toString(String format = nullptr) const;
  time_t toTimeStamp() const;
  int16_t getAbsoluteYear() const;

  bool operator< (DateTime other) const { return (uint64_t &)*this < (uint64_t &)other; }
  bool operator> (DateTime other) const { return (uint64_t &)*this > (uint64_t &)other; }
  bool operator<=(DateTime other) const { return (uint64_t &)*this <= (uint64_t &)other; }
  bool operator>=(DateTime other) const { return (uint64_t &)*this >= (uint64_t &)other; }
  bool operator==(DateTime other) const { return (uint64_t &)*this == (uint64_t &)other; }
  bool operator!=(DateTime other) const { return (uint64_t &)*this != (uint64_t &)other; }

protected:
  void timeStampToDateTime(time_t ti);
  void tmToDateTime(tm *pTm);
  tm dateTimeToTm() const;

  friend Variant epToVariant(const DateTime& dt);
  friend void epFromVariant(const Variant &variant, DateTime *pDt);
};

ptrdiff_t epStringify(Slice<char> buffer, String format, const DateTime &dt, const VarArg *epUnusedParam(pArgs));

Variant epToVariant(const DateTime& dt);
void epFromVariant(const Variant &variant, DateTime *pDt);

} // namespace ep

#endif // _EPDATETIME_HPP
