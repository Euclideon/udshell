#include "ep/epdatetime.h"
#include <time.h>

epDateTime::epDateTime() : epDateTime(time(nullptr)) {}

epDateTime::epDateTime(time_t ti)
{
  TimeStampToDateTime(ti);
}

epMutableString<64> epDateTime::ToString(epString format) const
{
  epMutableString<64> timeStr;
  if (format.empty())
    format = DEFAULT_DATETIME_STRING;

  tm t = DateTimeToTm();

  timeStr.reserve(format.length * 2);
  timeStr.length = strftime(timeStr.ptr, format.length * 2, format.toStringz(), &t);

  return timeStr;
}

time_t epDateTime::ToTimeStamp() const
{
  tm t;

  return mktime(&t);
}

void epDateTime::TimeStampToDateTime(time_t ti)
{
#if defined(EP_WINDOWS)
  tm t, *pTm = &t;
  localtime_s(&t, &ti);
#else
  tm *pTm = localtime(&ti);
#endif

  TmToDateTime(pTm);
}

void epDateTime::TmToDateTime(tm *pTm)
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

tm epDateTime::DateTimeToTm() const
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

int16_t epDateTime::GetAbsoluteYear() const
{
  return START_YEAR + (int16_t)year;
}

inline ptrdiff_t epStringify(epSlice<char> buffer, epString format, const epDateTime &dt, const epVarArg *epUnusedParam(pArgs))
{
  epSharedString out = dt.ToString(format);

  // if we're only counting
  if (!buffer.ptr)
    return out.length;

  // if the buffer is too small
  if (buffer.length < out.length)
    return buffer.length - out.length;

  out.copyTo(buffer);

  return out.length;
}

epVariant epToVariant(const epDateTime &dt)
{
  epVariant v;
  epKeyValuePair *pPairs = v.allocAssocArray(9);

  new (&pPairs[0]) epKeyValuePair("hour",  dt.hour);
  new (&pPairs[1]) epKeyValuePair("isdst", dt.isdst);
  new (&pPairs[2]) epKeyValuePair("mday",  dt.mday);
  new (&pPairs[3]) epKeyValuePair("min",   dt.min);
  new (&pPairs[4]) epKeyValuePair("mon",   dt.mon);
  new (&pPairs[5]) epKeyValuePair("sec",   dt.sec);
  new (&pPairs[6]) epKeyValuePair("wday",  dt.wday);
  new (&pPairs[7]) epKeyValuePair("yday",  dt.yday);
  new (&pPairs[8]) epKeyValuePair("year",  dt.year);

  return v;
}

void epFromVariant(const epVariant &variant, epDateTime *pDt)
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

