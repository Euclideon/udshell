#include "udPlatform.h"
#include "udPlatformUtil.h"

#include <string.h>
#include <stdarg.h>

#include <utility>

class udCString
{
  friend struct udString;
public:
  operator const char*() const { return pCStr; }
  ~udCString()
  {
    udFree(pCStr);
  }

private:
  const char *pCStr;
  udCString(udString str)
  {
    char *buf = (char*)udAlloc(str.length + 1);
    memcpy(buf, str.ptr, str.length);
    buf[str.length] = 0;
    pCStr = buf;
  }
};


// udString
inline udString::udString()
{}

inline udString::udString(const char *ptr, size_t length)
  : udSlice<const char>(ptr, length)
{}

template<typename C>
inline udString::udString(udSlice<C> rh)
  : udSlice<const char>(rh)
{}

inline udString::udString(const char *pString)
  : udSlice<const char>(pString, pString ? strlen(pString) : 0)
{}

template<size_t Len>
inline udString::udString(const char str[Len])
  : udSlice<const char>(str, N)
{}

inline udString& udString::operator =(udSlice<const char> rh)
{
  length = rh.length;
  ptr = rh.ptr;
  return *this;
}

inline udString& udString::operator =(const char *pString)
{
  ptr = pString;
  length = pString ? strlen(pString) : (size_t)0;
  return *this;
}

inline udString udString::slice(size_t first, size_t last) const
{
  UDASSERT(last <= length && first <= last, "Index out of range!");
  return udString(ptr + first, last - first);
}

inline bool udString::eq(udString rh) const
{
  return udSlice<const char>::eq(rh);
}
inline bool udString::eqi(udString rh) const
{
  if(length != rh.length)
    return false;
  for (size_t i = 0; i<length; ++i)
    if (toLower(ptr[i]) != toLower(rh.ptr[i]))
      return false;
  return true;
}

inline bool udString::beginsWith(udString rh) const
{
  return udSlice<const char>::beginsWith(rh);
}
inline bool udString::endsWith(udString rh) const
{
  return udSlice<const char>::endsWith(rh);
}
inline bool udString::beginsWithInsensitive(udString rh) const
{
  if (length < rh.length)
    return false;
  return slice(0, rh.length).eqi(rh);
}
inline bool udString::endsWithInsensitive(udString rh) const
{
  if (length < rh.length)
    return false;
  return slice(length - rh.length, length).eqi(rh);
}

inline char* udString::toStringz(char *pBuffer, size_t bufferLen) const
{
  size_t len = length < bufferLen-1 ? length : bufferLen-1;
  memcpy(pBuffer, ptr, len);
  pBuffer[len] = 0;
  return pBuffer;
}

inline udCString udString::toStringz() const
{
  return udCString(*this);
}

inline udString udString::trim(bool front, bool back) const
{
  size_t first = 0, last = length;
  if (front)
  {
    while (isWhitespace(ptr[first]) && first < length)
      ++first;
  }
  if (back)
  {
    while (last > first && isWhitespace(ptr[last - 1]))
      --last;
  }
  return slice(first, last);
}

template<bool skipEmptyTokens>
inline udString udString::popToken(udString delimiters)
{
  return udSlice<const char>::popToken<skipEmptyTokens>(delimiters);
}

template<bool skipEmptyTokens>
inline udSlice<udString> udString::tokenise(udSlice<udString> tokens, udString delimiters)
{
  return udSlice<const char>::tokenise<skipEmptyTokens>(tokens, delimiters);
}

inline uint32_t udString::hash(uint32_t hash) const
{
  size_t i = 0;
  while (i < length)
  {
    hash ^= (uint32_t)ptr[i++];
    hash *= 0x01000193;
  }
  return hash;
}


// udFixedString
template<size_t Size>
inline udFixedString<Size>::udFixedString()
{}

template<size_t Size>
inline udFixedString<Size>::udFixedString(udFixedString<Size> &&rval)
  : udFixedSlice<char, Size>(std::move(rval))
{}

template<size_t Size>
inline udFixedString<Size>::udFixedString(udFixedSlice<char, Size> &&rval)
  : udFixedSlice<char, Size>(std::move(rval))
{}

template<size_t Size>
template <typename U>
inline udFixedString<Size>::udFixedString(U *ptr, size_t length)
  : udFixedSlice<char, Size>(ptr, length)
{}

template<size_t Size>
template <typename U>
inline udFixedString<Size>::udFixedString(udSlice<U> slice)
  : udFixedSlice<char, Size>(slice)
{}

template<size_t Size>
inline udFixedString<Size>::udFixedString(const char *pString)
  : udFixedSlice<char, Size>(pString, pString ? strlen(pString) : 0)
{}

template<size_t Size>
inline udFixedString<Size>& udFixedString<Size>::operator =(udFixedString<Size> &&rval)
{
  udFixedSlice<char, Size>::operator=(std::move(rval));
  return *this;
}
template<size_t Size>
inline udFixedString<Size>& udFixedString<Size>::operator =(udFixedSlice<char, Size> &&rval)
{
  udFixedSlice<char, Size>::operator=(std::move(rval));
  return *this;
}
template<size_t Size>
template <typename U>
inline udFixedString<Size>& udFixedString<Size>::operator =(udSlice<U> rh)
{
  udFixedSlice<char, Size>::operator=(rh);
  return *this;
}
template<size_t Size>
inline udFixedString<Size>& udFixedString<Size>::operator =(const char *pString)
{
  udFixedSlice<char, Size>::operator=(pString, pString ? strlen(pString) : 0);
  return *this;
}

template<size_t Size>
inline udFixedString<Size> udFixedString<Size>::format(const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);

#if defined(_MSC_VER)
  size_t len = _vscprintf(pFormat, args) + 1;
#else
  size_t len = vsprintf(nullptr, pFormat, args) + 1;
#endif

  udFixedString<Size> r;
  r.length = len;
  if (len < Size)
  {
    r.numAllocated = 0;
    r.ptr = (char*)r.buffer;
  }
  else
  {
    r.numAllocated = udFixedSlice<char, Size>::numToAlloc(len + 1);
    r.ptr = (char*)udAlloc(sizeof(char) * r.numAllocated);
  }

#if defined(_MSC_VER)
  r.length = vsnprintf_s(r.ptr, len, len, pFormat, args);
#else
  r.length = vsnprintf(r.ptr, len, pFormat, args);
#endif

  va_end(args);

  return r;
}

template<size_t Size>
inline udCString udFixedString<Size>::toStringz() const
{
  return ((udString*)this)->toStringz();
}

template<size_t Size>
template<typename... Strings>
inline udFixedString<Size>& udFixedString<Size>::concat(const Strings&... strings)
{
  // flatten the args to an array
  udString args[] = { udString(strings)... };

  // call the (non-template) array version
  concat(args, UDARRAYSIZE(args));
  return *this;
}

template<size_t Size>
inline void udFixedString<Size>::concat(udString *pStrings, size_t numStrings)
{
  // calculate total length
  size_t len = this->length;
  for (size_t i = 0; i < numStrings; ++i)
    len += pStrings[i].length;

  this->reserve(len+1);

  // concatenate the strings
  char *pC = this->ptr + this->length;
  for (size_t i = 0; i < numStrings; ++i)
  {
    for (size_t j = 0; j < pStrings[i].length; ++j)
      *pC++ = pStrings[i].ptr[j];
  }
  *pC = 0; // null terminate for good measure

  this->length = len;
}


// udRCString
inline udRCString::udRCString()
{}

inline udRCString::udRCString(const udRCString &val)
  : udRCSlice<const char>(val)
{}


inline udRCString::udRCString(udRCString &&rval)
  : udRCSlice<const char>(std::move(rval))
{}

inline udRCString::udRCString(udRCSlice<const char> &&rval)
  : udRCSlice<const char>(std::move(rval))
{}

inline udRCString::udRCString(const udRCSlice<const char> &rcstr)
  : udRCSlice<const char>(rcstr)
{}

template <typename U>
inline udRCString::udRCString(U *ptr, size_t length)
  : udRCSlice<const char>(ptr, length)
{}

template <typename U>
inline udRCString::udRCString(udSlice<U> slice)
  : udRCSlice<const char>(slice)
{}

inline udRCString::udRCString(const char *pString)
  : udRCSlice<const char>(pString, pString ? strlen(pString) : 0)
{}

inline udRCString& udRCString::operator =(const udRCSlice<const char> &rh)
{
  udRCSlice<const char>::operator=(rh);
  return *this;
}
inline udRCString& udRCString::operator =(const udRCString &val)
{
  udRCSlice<const char>::operator=(val);
  return *this;
}

inline udRCString& udRCString::operator =(udRCString &&rval)
{
  udRCSlice<const char>::operator=(std::move(rval));
  return *this;
}
inline udRCString& udRCString::operator =(udRCSlice<const char> &&rval)
{
  udRCSlice<const char>::operator=(std::move(rval));
  return *this;
}
template <typename U>
inline udRCString& udRCString::operator =(udSlice<U> rh)
{
  *this = udRCSlice<U>(rh);
  return *this;
}
inline udRCString& udRCString::operator =(const char *pString)
{
  *this = udRCString(pString);
  return *this;
}

inline udRCString udRCString::slice(size_t first, size_t last) const
{
  UDASSERT(last <= length && first <= last, "Index out of range!");
  return udRCString(ptr + first, last - first, rc);
}

inline udCString udRCString::toStringz() const
{
  return ((udString*)this)->toStringz();
}

inline udRCString::udRCString(const char *ptr, size_t length, udRC *rc)
  : udRCSlice<const char>(ptr, length, rc)
{}

template<typename... Strings>
inline udRCString udRCString::concat(const Strings&... strings)
{
  // flatten the args to an array
  udString args[] = { udString(strings)... };

  // call the (non-template) array version
  return concat(args, sizeof(args) / sizeof(args[0]));
}




ptrdiff_t udStringify(udSlice<char> buffer, udString format, udString s);
ptrdiff_t udStringify(udSlice<char> buffer, udString format, const char *s);
ptrdiff_t udStringify(udSlice<char> buffer, udString format, bool b);
ptrdiff_t udStringify(udSlice<char> buffer, udString format, int64_t i);
ptrdiff_t udStringify(udSlice<char> buffer, udString format, uint64_t i);
ptrdiff_t udStringify(udSlice<char> buffer, udString format, double i);

struct udRCString::VarArg
{
  typedef ptrdiff_t(ProxyFunc)(udSlice<char>, udString, const void*);
  ProxyFunc *pProxy;
  const void *pArg;

  template<typename T>
  VarArg(const T& arg)
    : pProxy(&stringifyProxy<T>)
    , pArg(&arg)
  {}

  template<typename T> static ptrdiff_t stringifyProxy(udSlice<char> buffer, udString format, const void *pData)
  {
    return udStringify(buffer, format, *(T*)pData);
  }
};

// make the numeric types promote explicitly
template<> ptrdiff_t udRCString::VarArg::stringifyProxy<uint8_t>(udSlice<char> buffer, udString format, const void *pData);
template<> ptrdiff_t udRCString::VarArg::stringifyProxy<int8_t>(udSlice<char> buffer, udString format, const void *pData);
template<> ptrdiff_t udRCString::VarArg::stringifyProxy<uint16_t>(udSlice<char> buffer, udString format, const void *pData);
template<> ptrdiff_t udRCString::VarArg::stringifyProxy<int16_t>(udSlice<char> buffer, udString format, const void *pData);
template<> ptrdiff_t udRCString::VarArg::stringifyProxy<uint32_t>(udSlice<char> buffer, udString format, const void *pData);
template<> ptrdiff_t udRCString::VarArg::stringifyProxy<int32_t>(udSlice<char> buffer, udString format, const void *pData);
template<> ptrdiff_t udRCString::VarArg::stringifyProxy<float>(udSlice<char> buffer, udString format, const void *pData);

template<typename... Args>
inline udRCString udRCString::format(const char *pFormat, const Args&... args)
{
  // collect varargs into abstract arg list
  VarArg proxies[] = { VarArg(args)... };

  // call the internal function to do the work
  return formatInternal(pFormat, udSlice<VarArg>(proxies, UDARRAYSIZE(proxies)));
}
