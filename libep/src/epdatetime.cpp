#include "ep/cpp/datetime.h"
#include <time.h>

namespace ep {

DateTime::DateTime() : DateTime(time(nullptr)) {}

DateTime::DateTime(time_t ti)
{
  TimeStampToDateTime(ti);
}

MutableString64 DateTime::ToString(String format) const
{
  if (format.empty())
    format = DEFAULT_DATETIME_STRING;

  tm t = DateTimeToTm();

  MutableString64 timeStr(Reserve, format.length * 2);
  timeStr.length = strftime(timeStr.ptr, format.length * 2, format.toStringz(), &t);

  return timeStr;
}

time_t DateTime::ToTimeStamp() const
{
  tm t;

  return mktime(&t);
}

void DateTime::TimeStampToDateTime(time_t ti)
{
#if defined(EP_WINDOWS)
  tm t, *pTm = &t;
  localtime_s(&t, &ti);
#else
  tm *pTm = localtime(&ti);
#endif

  TmToDateTime(pTm);
}

void DateTime::TmToDateTime(tm *pTm)
{
  sec   = pTm->tm_sec;
  min   = pTm->tm_min;
  hour  = pTm->tm_hour;
  mday  = pTm->tm_mday;
  mon   = pTm->tm_mon;
  year  = pTm->tm_year + (0 - START_YEAR + 1900);
  wday  = pTm->tm_wday;
  yday  = pTm->tm_yday;
  isdst = pTm->tm_isdst;
}

tm DateTime::DateTimeToTm() const
{
  tm t;

  t.tm_sec   = sec;
  t.tm_min   = min;
  t.tm_hour  = hour;
  t.tm_mday  = mday;
  t.tm_mon   = mon;
  t.tm_year  = (int16_t)year - (0 - START_YEAR + 1900);
  t.tm_wday  = wday;
  t.tm_yday  = yday;
  t.tm_isdst = isdst;

  return t;
}

int16_t DateTime::GetAbsoluteYear() const
{
  return START_YEAR + (int16_t)year;
}

inline ptrdiff_t epStringify(Slice<char> buffer, String format, const DateTime &dt, const VarArg *epUnusedParam(pArgs))
{
  SharedString out = dt.ToString(format);

  // if we're only counting
  if (!buffer.ptr)
    return out.length;

  // if the buffer is too small
  if (buffer.length < out.length)
    return buffer.length - out.length;

  out.copyTo(buffer);

  return out.length;
}

Variant epToVariant(const DateTime &dt)
{
  Array<KeyValuePair> pairs(Reserve, 9);
  pairs.pushBack(KeyValuePair("hour",  dt.hour));
  pairs.pushBack(KeyValuePair("isdst", dt.isdst));
  pairs.pushBack(KeyValuePair("mday",  dt.mday));
  pairs.pushBack(KeyValuePair("min",   dt.min));
  pairs.pushBack(KeyValuePair("mon",   dt.mon));
  pairs.pushBack(KeyValuePair("sec",   dt.sec));
  pairs.pushBack(KeyValuePair("wday",  dt.wday));
  pairs.pushBack(KeyValuePair("yday",  dt.yday));
  pairs.pushBack(KeyValuePair("year",  dt.year));
  return std::move(pairs);
}

void epFromVariant(const Variant &variant, DateTime *pDt)
{
  pDt->hour   = variant["hour"].as<uint8_t>();
  pDt->isdst  = variant["isdst"].as<bool>();
  pDt->mday   = variant["mday"].as<uint8_t>();
  pDt->min    = variant["min"].as<uint8_t>();
  pDt->mon    = variant["mon"].as<uint8_t>();
  pDt->sec    = variant["sec"].as<uint8_t>();
  pDt->wday   = variant["wday"].as<uint8_t>();
  pDt->yday   = variant["yday"].as<uint16_t>();
  pDt->year   = variant["year"].as<uint16_t>();
}

} // namespace ep

