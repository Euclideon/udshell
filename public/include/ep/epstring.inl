#include "ep/epplatform.h"

#include <string.h>
#include <stdarg.h>

#include <utility>


class epCString
{
  friend struct epString;
public:
  operator const char*() const { return pCStr; }
  ~epCString()
  {
    udFree(pCStr);
  }

private:
  const char *pCStr;
  epCString(epString str)
  {
    char *buf = (char*)udAlloc(str.length + 1);
    memcpy(buf, str.ptr, str.length);
    buf[str.length] = 0;
    pCStr = buf;
  }
};


// epString
inline epString::epString()
{}

inline epString::epString(const char *ptr, size_t length)
  : epSlice<const char>(ptr, length)
{}

template<typename C>
inline epString::epString(epSlice<C> rh)
  : epSlice<const char>(rh)
{}

inline epString::epString(const char *pString)
  : epSlice<const char>(pString, pString ? strlen(pString) : 0)
{}

template<size_t N>
inline epString::epString(const char str[N])
  : epSlice<const char>(str, N)
{}

inline epString& epString::operator =(epSlice<const char> rh)
{
  length = rh.length;
  ptr = rh.ptr;
  return *this;
}

inline epString& epString::operator =(const char *pString)
{
  ptr = pString;
  length = pString ? strlen(pString) : (size_t)0;
  return *this;
}

inline epString epString::slice(ptrdiff_t first, ptrdiff_t last) const
{
  size_t start = (size_t)(first < 0 ? first + length : first);
  size_t end = (size_t)(last < 0 ? last + length : last);
  EPASSERT(end <= length && start <= end, "Index out of range!");
  return epString(ptr + start, end - start);
}

inline bool epString::eq(epString rh) const
{
  return epSlice<const char>::eq(rh);
}
inline bool epString::eqIC(epString rh) const
{
  if(length != rh.length)
    return false;
  for (size_t i = 0; i<length; ++i)
    if (toLower(ptr[i]) != toLower(rh.ptr[i]))
      return false;
  return true;
}
inline bool epString::beginsWith(epString rh) const
{
  return epSlice<const char>::beginsWith(rh);
}
inline bool epString::beginsWithIC(epString rh) const
{
  if (length < rh.length)
    return false;
  return slice(0, rh.length).eqIC(rh);
}
inline bool epString::endsWith(epString rh) const
{
  return epSlice<const char>::endsWith(rh);
}
inline bool epString::endsWithIC(epString rh) const
{
  if (length < rh.length)
    return false;
  return slice(length - rh.length, length).eqIC(rh);
}

inline ptrdiff_t epString::cmp(epString rh) const
{
  return epSlice<const char>::cmp(rh);
}
inline ptrdiff_t epString::cmpIC(epString rh) const
{
  size_t len = length < rh.length ? length : rh.length;
  for (size_t i = 0; i < len; ++i)
  {
    char a = toLower(ptr[i]), b = toLower(rh.ptr[i]);
    if (a == b)
      continue;
    return a < b ? -1 : 1;
  }
  return length - rh.length;
}

inline char* epString::toStringz(char *pBuffer, size_t bufferLen) const
{
  size_t len = length < bufferLen-1 ? length : bufferLen-1;
  memcpy(pBuffer, ptr, len);
  pBuffer[len] = 0;
  return pBuffer;
}

inline epCString epString::toStringz() const
{
  return epCString(*this);
}

inline size_t epString::findFirstIC(epString s) const
{
  if (s.empty())
    return 0;
  ptrdiff_t len = length-s.length;
  for (ptrdiff_t i = 0; i < len; ++i)
  {
    size_t j = 0;
    for (; j < s.length; ++j)
    {
      if (toLower(ptr[i + j]) != toLower(s.ptr[j]))
        break;
    }
    if (j == s.length)
      return i;
  }
  return length;
}
inline size_t epString::findLastIC(epString s) const
{
  if (s.empty())
    return length;
  for (ptrdiff_t i = length-s.length; i >= 0; --i)
  {
    size_t j = 0;
    for (; j < s.length; ++j)
    {
      if (toLower(ptr[i + j]) != toLower(s.ptr[j]))
        break;
    }
    if (j == s.length)
      return i;
  }
  return length;
}

inline epString epString::getLeftAtFirstIC(epString s, bool bInclusive) const
{
  return slice(0, findFirstIC(s) + (bInclusive ? s.length : 0));
}
inline epString epString::getLeftAtLastIC(epString s, bool bInclusive) const
{
  return slice(0, findLastIC(s) + (bInclusive ? s.length : 0));
}
inline epString epString::getRightAtFirstIC(epString s, bool bInclusive) const
{
  return slice(findFirstIC(s) + (bInclusive ? 0 : s.length), length);
}
inline epString epString::getRightAtLastIC(epString s, bool bInclusive) const
{
  return slice(findLastIC(s) + (bInclusive ? 0 : s.length), length);
}

inline epString epString::trim(bool front, bool back) const
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
inline epString epString::popToken(epString delimiters)
{
  return epSlice<const char>::popToken<skipEmptyTokens>(delimiters);
}

template<bool skipEmptyTokens>
inline epSlice<epString> epString::tokenise(epSlice<epString> tokens, epString delimiters)
{
  return epSlice<const char>::tokenise<skipEmptyTokens>(tokens, delimiters);
}

inline uint32_t epString::hash(uint32_t hash) const
{
  size_t i = 0;
  while (i < length)
  {
    hash ^= (uint32_t)ptr[i++];
    hash *= 0x01000193;
  }
  return hash;
}


// epMutableString
template<size_t Size>
inline epMutableString<Size>::epMutableString()
{}

template<size_t Size>
inline epMutableString<Size>::epMutableString(epMutableString<Size> &&rval)
  : epArray<char, Size>(std::move(rval))
{}

template<size_t Size>
inline epMutableString<Size>::epMutableString(epArray<char, Size> &&rval)
  : epArray<char, Size>(std::move(rval))
{}

template<size_t Size>
template <typename U>
inline epMutableString<Size>::epMutableString(U *ptr, size_t length)
  : epArray<char, Size>(ptr, length)
{}

template<size_t Size>
template <typename U>
inline epMutableString<Size>::epMutableString(epSlice<U> slice)
  : epArray<char, Size>(slice)
{}

template<size_t Size>
inline epMutableString<Size>::epMutableString(const char *pString)
  : epArray<char, Size>(pString, pString ? strlen(pString) : 0)
{}

template<size_t Size>
inline epMutableString<Size>& epMutableString<Size>::operator =(epMutableString<Size> &&rval)
{
  epArray<char, Size>::operator=(std::move(rval));
  return *this;
}
template<size_t Size>
inline epMutableString<Size>& epMutableString<Size>::operator =(epArray<char, Size> &&rval)
{
  epArray<char, Size>::operator=(std::move(rval));
  return *this;
}
template<size_t Size>
template <typename U>
inline epMutableString<Size>& epMutableString<Size>::operator =(epSlice<U> rh)
{
  epArray<char, Size>::operator=(rh);
  return *this;
}
template<size_t Size>
inline epMutableString<Size>& epMutableString<Size>::operator =(const char *pString)
{
  epArray<char, Size>::operator=(epString(pString, pString ? strlen(pString) : 0));
  return *this;
}

template<size_t Size>
inline epMutableString<Size>& epMutableString<Size>::sprintf(const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);

#if defined(_MSC_VER)
  size_t len = _vscprintf(pFormat, args) + 1;
#else
  size_t len = vsprintf(nullptr, pFormat, args) + 1;
#endif

  this->reserve(len + 1);

#if defined(_MSC_VER)
  this->length = vsnprintf_s(this->ptr, len, len, pFormat, args);
#else
  this->length = vsnprintf(this->ptr, len, pFormat, args);
#endif

  va_end(args);

  return *this;
}

template<size_t Size>
inline epMutableString<Size>& epMutableString<Size>::toUpper()
{
  for (size_t i = 0; i < this->length; ++i)
  {
    if (isAlpha(this->ptr[i]))
      this->ptr[i] == toUpper(this->ptr[i]);
  }
}
template<size_t Size>
inline epMutableString<Size>& epMutableString<Size>::toLower()
{
  for (size_t i = 0; i < this->length; ++i)
  {
    if (isAlpha(this->ptr[i]))
      this->ptr[i] == toLower(this->ptr[i]);
  }
}

template<size_t Size>
inline epCString epMutableString<Size>::toStringz() const
{
  return ((epString*)this)->toStringz();
}


// epSharedString
inline epSharedString::epSharedString()
{}

inline epSharedString::epSharedString(const epSharedString &val)
  : epSharedSlice<const char>(val)
{}


inline epSharedString::epSharedString(epSharedString &&rval)
  : epSharedSlice<const char>(std::move(rval))
{}

inline epSharedString::epSharedString(epSharedSlice<const char> &&rval)
  : epSharedSlice<const char>(std::move(rval))
{}

inline epSharedString::epSharedString(const epSharedSlice<const char> &rcstr)
  : epSharedSlice<const char>(rcstr)
{}

template <typename U>
inline epSharedString::epSharedString(U *ptr, size_t length)
  : epSharedSlice<const char>(ptr, length)
{}

template <typename U>
inline epSharedString::epSharedString(epSlice<U> slice)
  : epSharedSlice<const char>(slice)
{}

inline epSharedString::epSharedString(const char *pString)
  : epSharedSlice<const char>(pString, pString ? strlen(pString) : 0)
{}

inline epSharedString& epSharedString::operator =(const epSharedSlice<const char> &rh)
{
  epSharedSlice<const char>::operator=(rh);
  return *this;
}
inline epSharedString& epSharedString::operator =(const epSharedString &val)
{
  epSharedSlice<const char>::operator=(val);
  return *this;
}

inline epSharedString& epSharedString::operator =(epSharedString &&rval)
{
  epSharedSlice<const char>::operator=(std::move(rval));
  return *this;
}
inline epSharedString& epSharedString::operator =(epSharedSlice<const char> &&rval)
{
  epSharedSlice<const char>::operator=(std::move(rval));
  return *this;
}
template <typename U>
inline epSharedString& epSharedString::operator =(epSlice<U> rh)
{
  *this = epSharedSlice<U>(rh);
  return *this;
}
inline epSharedString& epSharedString::operator =(const char *pString)
{
  *this = epSharedString(pString);
  return *this;
}

inline epSharedString epSharedString::slice(ptrdiff_t first, ptrdiff_t last) const
{
  epString s = ((epString*)this)->slice(first, last);
  return epSharedString(s.ptr, s.length, rc);
}

inline epCString epSharedString::toStringz() const
{
  return ((epString*)this)->toStringz();
}

inline epSharedString epSharedString::asUpper() const
{
  EPASSERT(false, "TODO");
  return nullptr;
}
inline epSharedString epSharedString::asLower() const
{
  EPASSERT(false, "TODO");
  return nullptr;
}

inline epSharedString epSharedString::getLeftAtFirstIC(epString s, bool bInclusive) const
{
  return slice(0, findFirstIC(s) + (bInclusive ? s.length : 0));
}
inline epSharedString epSharedString::getLeftAtLastIC(epString s, bool bInclusive) const
{
  return slice(0, findLastIC(s) + (bInclusive ? s.length : 0));
}
inline epSharedString epSharedString::getRightAtFirstIC(epString s, bool bInclusive) const
{
  return slice(findFirstIC(s) + (bInclusive ? 0 : s.length), length);
}
inline epSharedString epSharedString::getRightAtLastIC(epString s, bool bInclusive) const
{
  return slice(findLastIC(s) + (bInclusive ? 0 : s.length), length);
}

inline epSharedString epSharedString::trim(bool front, bool back) const
{
  epString s = ((epString*)this)->trim(front, back);
  return epSharedString(s.ptr, s.length, rc);
}

template<bool skipEmptyTokens>
inline epSharedString epSharedString::popToken(epString delimiters)
{
  epString s = ((epString*)this)->popToken<skipEmptyTokens>(delimiters);
  return epSharedString(s.ptr, s.length, rc);
}
template<bool skipEmptyTokens>
inline epSlice<epSharedString> epSharedString::tokenise(epSlice<epSharedString> tokens, epString delimiters) const
{
  epArray<epString, 64> t;
  t.reserve(tokens.length);
  t.length = tokens.length;
  epSlice<epString> tok = ((epString*)this)->tokenise<skipEmptyTokens>(t, delimiters);
  for (size_t i = 0; i < tok.length; ++i)
    new(&tokens.ptr[i]) epSharedString(tok.ptr[i].ptr, tok.ptr[i].length, rc);
  return tokens.slice(0, tok.length);
}

inline epSharedString::epSharedString(const char *ptr, size_t length, epRC *rc)
  : epSharedSlice<const char>(ptr, length, rc)
{}


//
// varargs functions....
//

ptrdiff_t epStringify(epSlice<char> buffer, epString format, nullptr_t, const epVarArg *pArgs);
ptrdiff_t epStringify(epSlice<char> buffer, epString format, epString s, const epVarArg *pArgs);
inline ptrdiff_t epStringify(epSlice<char> buffer, epString format, const char *s, const epVarArg *pArgs) { return epStringify(buffer, format, epString(s, s ? strlen(s) : 0), pArgs); }
template<size_t N>
inline ptrdiff_t epStringify(epSlice<char> buffer, epString format, const char s[N], const epVarArg *pArgs) { return epStringify(buffer, format, epString(s, N-1), pArgs); }
ptrdiff_t epStringify(epSlice<char> buffer, epString format, bool b, const epVarArg *pArgs);
ptrdiff_t epStringify(epSlice<char> buffer, epString format, int64_t i, const epVarArg *pArgs);
ptrdiff_t epStringify(epSlice<char> buffer, epString format, uint64_t i, const epVarArg *pArgs);
ptrdiff_t epStringify(epSlice<char> buffer, epString format, double i, const epVarArg *pArgs);
template<typename T>
ptrdiff_t epStringify(epSlice<char> buffer, epString format, epSlice<T> arr, const epVarArg *pArgs)
{
  ptrdiff_t len = 2; // for the "[" .. "]"
  len += arr.length > 1 ? (arr.length-1)*2 : 0; // bytes for the ", " separator sequences

  if (!buffer.ptr)
  {
    for (auto &v : arr)
      len += epStringifyTemplate(nullptr, format, v, pArgs);
  }
  else
  {
    if (buffer.length < (size_t)len)
      return -len;

    buffer[0] = '['; buffer.popFront();
    for (size_t i = 0; i < arr.length; ++i)
    {
      if (i > 0)
      {
        buffer[0] = ','; buffer[1] = ' ';
        buffer.pop(2);
      }
      ptrdiff_t l = epStringifyTemplate(buffer, format, arr[i], pArgs);
      if (l < 0)
        return l - len;
      buffer.pop(l);
      len += l;
    }
    buffer[0] = ']';
  }
  return len;
}


// stringify helper
namespace ud_internal
{
  template<typename T>
  struct StringifyProxy
  {
    inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs)
    {
      return epStringify(buffer, format, *(T*)pData, pArgs);
    }
    static const size_t intify = 0;
  };

  // make the numeric types promote explicitly
  template<> struct StringifyProxy<uint8_t>  { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, (uint64_t)*(uint8_t*)pData, pArgs); }  inline static int64_t intify(const void *pData) { return (int64_t)*(uint8_t*)pData; } };
  template<> struct StringifyProxy<int8_t>   { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, (int64_t)*(int8_t*)pData, pArgs); }    inline static int64_t intify(const void *pData) { return (int64_t)*(int8_t*)pData; } };
  template<> struct StringifyProxy<uint16_t> { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, (uint64_t)*(uint16_t*)pData, pArgs); } inline static int64_t intify(const void *pData) { return (int64_t)*(uint16_t*)pData; } };
  template<> struct StringifyProxy<int16_t>  { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, (int64_t)*(int16_t*)pData, pArgs); }   inline static int64_t intify(const void *pData) { return (int64_t)*(int16_t*)pData; } };
  template<> struct StringifyProxy<uint32_t> { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, (uint64_t)*(uint32_t*)pData, pArgs); } inline static int64_t intify(const void *pData) { return (int64_t)*(uint32_t*)pData; } };
  template<> struct StringifyProxy<int32_t>  { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, (int64_t)*(int32_t*)pData, pArgs); }   inline static int64_t intify(const void *pData) { return (int64_t)*(int32_t*)pData; } };
  template<> struct StringifyProxy<uint64_t> { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, *(uint64_t*)pData, pArgs); }           inline static int64_t intify(const void *pData) { return (int64_t)*(uint64_t*)pData; } };
  template<> struct StringifyProxy<int64_t>  { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, *(int64_t*)pData, pArgs); }            inline static int64_t intify(const void *pData) { return *(int64_t*)pData; } };
  template<> struct StringifyProxy<float>    { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, (double)*(float*)pData, pArgs); }      inline static int64_t intify(const void *pData) { return (int64_t)*(float*)pData; } };
  template<> struct StringifyProxy<double>   { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, *(double*)pData, pArgs); }             inline static int64_t intify(const void *pData) { return (int64_t)*(double*)pData; } };
  template<size_t N>
  struct StringifyProxy<const char[N]>       { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, epString((const char*)pData, N-1), pArgs); } static const size_t intify = 0; };
  template<size_t N>
  struct StringifyProxy<epMutableString<N>>  { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, *(epString*)pData, pArgs); } static const size_t intify = 0; };
  template<> struct StringifyProxy<epSharedString>  { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, *(epString*)pData, pArgs); } static const size_t intify = 0; };

  size_t getLength(epSlice<epVarArg> args);
  epSlice<char> concatenate(epSlice<char> buffer, epSlice<epVarArg> args);
  epSlice<char> format(epString format, epSlice<char> buffer, epSlice<epVarArg> args);
}

template<typename T>
epVarArg::epVarArg(const T& arg)
: pArg(&arg)
, pStringProxy(ud_internal::StringifyProxy<T>::stringify)
, pIntProxy(ud_internal::StringifyProxy<T>::intify)
{}


template<typename T>
inline ptrdiff_t epStringifyTemplate(epSlice<char> buffer, epString format, const T &val, const epVarArg *pArgs)
{
  return ud_internal::StringifyProxy<T>::stringify(buffer, format, (void*)&val, pArgs);
}


// epMutableString

template<size_t Size>
template<typename... Args>
inline epMutableString<Size>& epMutableString<Size>::concat(const Args&... args)
{
  using namespace ud_internal;
  this->clear();
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  appendInternal(epSlice<epVarArg>(proxies, sizeof...(Args)));
  return *this;
}
template<size_t Size>
template<typename... Args>
inline epMutableString<Size>& epMutableString<Size>::append(const Args&... args)
{
  using namespace ud_internal;
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  appendInternal(epSlice<epVarArg>(proxies, sizeof...(Args)));
  return *this;
}
template<size_t Size>
template<typename... Args>
inline epMutableString<Size>& epMutableString<Size>::format(epString format, const Args&... args)
{
  using namespace ud_internal;
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  formatInternal(format, epSlice<epVarArg>(proxies, sizeof...(Args)));
  return *this;
}

template<size_t Size>
inline void epMutableString<Size>::appendInternal(epSlice<epVarArg> args)
{
  using namespace ud_internal;
  size_t len = getLength(args);
  this->reserve(this->length + len + 1);
  ud_internal::concatenate(epSlice<char>(this->ptr + this->length, len), args);
  this->length += len;
  this->ptr[this->length] = 0;
}
template<size_t Size>
inline void epMutableString<Size>::formatInternal(epString format, epSlice<epVarArg> args)
{
  using namespace ud_internal;
  size_t len = ud_internal::format(format, nullptr, args).length;
  this->reserve(len + 1);
  ud_internal::format(format, epSlice<char>(this->ptr, len), args);
  this->length = len;
  this->ptr[len] = 0;
}


// epSharedString

template<typename... Args>
inline epSharedString epSharedString::concat(const Args&... args)
{
  using namespace ud_internal;
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  return concatInternal(epSlice<epVarArg>(proxies, sizeof...(Args)));
}
template<typename... Args>
inline epSharedString epSharedString::format(epString format, const Args&... args)
{
  using namespace ud_internal;
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  return formatInternal(format, epSlice<epVarArg>(proxies, sizeof...(Args)));
}
