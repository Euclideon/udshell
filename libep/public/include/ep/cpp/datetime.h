#pragma once
#if !defined(_EPDATETIME_HPP)
#define _EPDATETIME_HPP

#include "ep/c/datetime.h"
#include "ep/cpp/variant.h"

namespace ep {

class DateTime : public epDateTime
{
public:
  DateTime();
  DateTime(time_t ti);

  MutableString64 ToString(String format = nullptr) const;
  time_t ToTimeStamp() const;
  int16_t GetAbsoluteYear() const;

  bool operator< (DateTime other) const { return (uint64_t &)*this < (uint64_t &)other; }
  bool operator> (DateTime other) const { return (uint64_t &)*this > (uint64_t &)other; }
  bool operator<=(DateTime other) const { return (uint64_t &)*this <= (uint64_t &)other; }
  bool operator>=(DateTime other) const { return (uint64_t &)*this >= (uint64_t &)other; }
  bool operator==(DateTime other) const { return (uint64_t &)*this == (uint64_t &)other; }
  bool operator!=(DateTime other) const { return (uint64_t &)*this != (uint64_t &)other; }

protected:
  void TimeStampToDateTime(time_t ti);
  void TmToDateTime(tm *pTm);
  tm DateTimeToTm() const;

  friend Variant epToVariant(const DateTime& dt);
  friend void epFromVariant(const Variant &variant, DateTime *pDt);
};

} // namespace ep

ptrdiff_t epStringify(Slice<char> buffer, String format, const DateTime &dt, const epVarArg *epUnusedParam(pArgs));

Variant epToVariant(const DateTime& dt);
void epFromVariant(const Variant &variant, DateTime *pDt);

#endif // _EPDATETIME_HPP
