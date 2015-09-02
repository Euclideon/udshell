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

inline udString udString::slice(ptrdiff_t first, ptrdiff_t last) const
{
  size_t start = (size_t)(first < 0 ? first + length : first);
  size_t end = (size_t)(last < 0 ? last + length : last);
  UDASSERT(end <= length && start <= end, "Index out of range!");
  return udString(ptr + start, end - start);
}

inline bool udString::eq(udString rh) const
{
  return udSlice<const char>::eq(rh);
}
inline bool udString::eqIC(udString rh) const
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
inline bool udString::beginsWithIC(udString rh) const
{
  if (length < rh.length)
    return false;
  return slice(0, rh.length).eqIC(rh);
}
inline bool udString::endsWith(udString rh) const
{
  return udSlice<const char>::endsWith(rh);
}
inline bool udString::endsWithIC(udString rh) const
{
  if (length < rh.length)
    return false;
  return slice(length - rh.length, length).eqIC(rh);
}

inline ptrdiff_t udString::cmp(udString rh) const
{
  return udSlice<const char>::cmp(rh);
}
inline ptrdiff_t udString::cmpIC(udString rh) const
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

inline size_t udString::findFirstIC(udString s) const
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
inline size_t udString::findLastIC(udString s) const
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

inline udString udString::getLeftAtFirstIC(udString s, bool bInclusive) const
{
  return slice(0, findFirstIC(s) + (bInclusive ? s.length : 0));
}
inline udString udString::getLeftAtLastIC(udString s, bool bInclusive) const
{
  return slice(0, findLastIC(s) + (bInclusive ? s.length : 0));
}
inline udString udString::getRightAtFirstIC(udString s, bool bInclusive) const
{
  return slice(findFirstIC(s) + (bInclusive ? 0 : s.length), length);
}
inline udString udString::getRightAtLastIC(udString s, bool bInclusive) const
{
  return slice(findLastIC(s) + (bInclusive ? 0 : s.length), length);
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


// udMutableString
template<size_t Size>
inline udMutableString<Size>::udMutableString()
{}

template<size_t Size>
inline udMutableString<Size>::udMutableString(udMutableString<Size> &&rval)
  : udFixedSlice<char, Size>(std::move(rval))
{}

template<size_t Size>
inline udMutableString<Size>::udMutableString(udFixedSlice<char, Size> &&rval)
  : udFixedSlice<char, Size>(std::move(rval))
{}

template<size_t Size>
template <typename U>
inline udMutableString<Size>::udMutableString(U *ptr, size_t length)
  : udFixedSlice<char, Size>(ptr, length)
{}

template<size_t Size>
template <typename U>
inline udMutableString<Size>::udMutableString(udSlice<U> slice)
  : udFixedSlice<char, Size>(slice)
{}

template<size_t Size>
inline udMutableString<Size>::udMutableString(const char *pString)
  : udFixedSlice<char, Size>(pString, pString ? strlen(pString) : 0)
{}

template<size_t Size>
inline udMutableString<Size>& udMutableString<Size>::operator =(udMutableString<Size> &&rval)
{
  udFixedSlice<char, Size>::operator=(std::move(rval));
  return *this;
}
template<size_t Size>
inline udMutableString<Size>& udMutableString<Size>::operator =(udFixedSlice<char, Size> &&rval)
{
  udFixedSlice<char, Size>::operator=(std::move(rval));
  return *this;
}
template<size_t Size>
template <typename U>
inline udMutableString<Size>& udMutableString<Size>::operator =(udSlice<U> rh)
{
  udFixedSlice<char, Size>::operator=(rh);
  return *this;
}
template<size_t Size>
inline udMutableString<Size>& udMutableString<Size>::operator =(const char *pString)
{
  udFixedSlice<char, Size>::operator=(udString(pString, pString ? strlen(pString) : 0));
  return *this;
}

template<size_t Size>
inline udMutableString<Size>& udMutableString<Size>::sprintf(const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);

#if defined(_MSC_VER)
  size_t len = _vscprintf(pFormat, args) + 1;
#else
  size_t len = vsprintf(nullptr, pFormat, args) + 1;
#endif

  reserve(len + 1);

#if defined(_MSC_VER)
  length = vsnprintf_s(ptr, len, len, pFormat, args);
#else
  length = vsnprintf(ptr, len, pFormat, args);
#endif

  va_end(args);

  return *this;
}

template<size_t Size>
inline udMutableString<Size>& udMutableString<Size>::toUpper()
{
  for (size_t i = 0; i < length; ++i)
  {
    if (isAlpha(ptr[i]))
      ptr[i] == toUpper(ptr[i]);
  }
}
template<size_t Size>
inline udMutableString<Size>& udMutableString<Size>::toLower()
{
  for (size_t i = 0; i < length; ++i)
  {
    if (isAlpha(ptr[i]))
      ptr[i] == toLower(ptr[i]);
  }
}

template<size_t Size>
inline udCString udMutableString<Size>::toStringz() const
{
  return ((udString*)this)->toStringz();
}


// udSharedString
inline udSharedString::udSharedString()
{}

inline udSharedString::udSharedString(const udSharedString &val)
  : udSharedSlice<const char>(val)
{}


inline udSharedString::udSharedString(udSharedString &&rval)
  : udSharedSlice<const char>(std::move(rval))
{}

inline udSharedString::udSharedString(udSharedSlice<const char> &&rval)
  : udSharedSlice<const char>(std::move(rval))
{}

inline udSharedString::udSharedString(const udSharedSlice<const char> &rcstr)
  : udSharedSlice<const char>(rcstr)
{}

template <typename U>
inline udSharedString::udSharedString(U *ptr, size_t length)
  : udSharedSlice<const char>(ptr, length)
{}

template <typename U>
inline udSharedString::udSharedString(udSlice<U> slice)
  : udSharedSlice<const char>(slice)
{}

inline udSharedString::udSharedString(const char *pString)
  : udSharedSlice<const char>(pString, pString ? strlen(pString) : 0)
{}

inline udSharedString& udSharedString::operator =(const udSharedSlice<const char> &rh)
{
  udSharedSlice<const char>::operator=(rh);
  return *this;
}
inline udSharedString& udSharedString::operator =(const udSharedString &val)
{
  udSharedSlice<const char>::operator=(val);
  return *this;
}

inline udSharedString& udSharedString::operator =(udSharedString &&rval)
{
  udSharedSlice<const char>::operator=(std::move(rval));
  return *this;
}
inline udSharedString& udSharedString::operator =(udSharedSlice<const char> &&rval)
{
  udSharedSlice<const char>::operator=(std::move(rval));
  return *this;
}
template <typename U>
inline udSharedString& udSharedString::operator =(udSlice<U> rh)
{
  *this = udSharedSlice<U>(rh);
  return *this;
}
inline udSharedString& udSharedString::operator =(const char *pString)
{
  *this = udSharedString(pString);
  return *this;
}

inline udSharedString udSharedString::slice(ptrdiff_t first, ptrdiff_t last) const
{
  udString s = ((udString*)this)->slice(first, last);
  return udSharedString(s.ptr, s.length, rc);
}

inline udCString udSharedString::toStringz() const
{
  return ((udString*)this)->toStringz();
}

inline udSharedString udSharedString::asUpper() const
{
  UDASSERT(false, "TODO");
  return nullptr;
}
inline udSharedString udSharedString::asLower() const
{
  UDASSERT(false, "TODO");
  return nullptr;
}

inline udSharedString udSharedString::getLeftAtFirstIC(udString s, bool bInclusive) const
{
  return slice(0, findFirstIC(s) + (bInclusive ? s.length : 0));
}
inline udSharedString udSharedString::getLeftAtLastIC(udString s, bool bInclusive) const
{
  return slice(0, findLastIC(s) + (bInclusive ? s.length : 0));
}
inline udSharedString udSharedString::getRightAtFirstIC(udString s, bool bInclusive) const
{
  return slice(findFirstIC(s) + (bInclusive ? 0 : s.length), length);
}
inline udSharedString udSharedString::getRightAtLastIC(udString s, bool bInclusive) const
{
  return slice(findLastIC(s) + (bInclusive ? 0 : s.length), length);
}

inline udSharedString udSharedString::trim(bool front, bool back) const
{
  udString s = ((udString*)this)->trim(front, back);
  return udSharedString(s.ptr, s.length, rc);
}

template<bool skipEmptyTokens>
inline udSharedString udSharedString::popToken(udString delimiters)
{
  udString s = ((udString*)this)->popToken<skipEmptyTokens>(delimiters);
  return udSharedString(s.ptr, s.length, rc);
}
template<bool skipEmptyTokens>
inline udSlice<udSharedString> udSharedString::tokenise(udSlice<udSharedString> tokens, udString delimiters) const
{
  udFixedSlice<udString, 64> t;
  t.reserve(tokens.length);
  t.length = tokens.length;
  udSlice<udString> tok = ((udString*)this)->tokenise<skipEmptyTokens>(t, delimiters);
  for (size_t i = 0; i < tok.length; ++i)
    new(&tokens.ptr[i]) udSharedString(tok.ptr[i].ptr, tok.ptr[i].length, rc);
  return tokens.slice(0, tok.length);
}

inline udSharedString::udSharedString(const char *ptr, size_t length, udRC *rc)
  : udSharedSlice<const char>(ptr, length, rc)
{}


//
// varargs functions....
//

ptrdiff_t udStringify(udSlice<char> buffer, udString format, udString s);
ptrdiff_t udStringify(udSlice<char> buffer, udString format, const char *s);
template<size_t N>
inline ptrdiff_t udStringify(udSlice<char> buffer, udString format, const char s[N]) { return udStringify(buffer, format, udString(s, N-1)); }
ptrdiff_t udStringify(udSlice<char> buffer, udString format, bool b);
ptrdiff_t udStringify(udSlice<char> buffer, udString format, int64_t i);
ptrdiff_t udStringify(udSlice<char> buffer, udString format, uint64_t i);
ptrdiff_t udStringify(udSlice<char> buffer, udString format, double i);

// stringify helper
namespace ud_internal
{
  // vararg for stringification
  struct VarArg
  {
    typedef ptrdiff_t(ProxyFunc)(udSlice<char>, udString, const void*);
    ProxyFunc *pProxy;
    const void *pArg;

    template<typename T>
    VarArg(const T& arg)
      : pProxy(&StringifyProxy<T>::stringify)
      , pArg(&arg)
    {}

    template<typename T>
    struct StringifyProxy
    {
      inline static ptrdiff_t stringify(udSlice<char> buffer, udString format, const void *pData)
      {
        return udStringify(buffer, format, *(T*)pData);
      }
    };
  };
  // make the numeric types promote explicitly
  template<> struct VarArg::StringifyProxy<uint8_t>  { inline static ptrdiff_t stringify(udSlice<char> buffer, udString format, const void *pData) { return udStringify(buffer, format, (uint64_t)*(uint8_t*)pData); } };
  template<> struct VarArg::StringifyProxy<int8_t>   { inline static ptrdiff_t stringify(udSlice<char> buffer, udString format, const void *pData) { return udStringify(buffer, format, (int64_t)*(int8_t*)pData); } };
  template<> struct VarArg::StringifyProxy<uint16_t> { inline static ptrdiff_t stringify(udSlice<char> buffer, udString format, const void *pData) { return udStringify(buffer, format, (uint64_t)*(uint16_t*)pData); } };
  template<> struct VarArg::StringifyProxy<int16_t>  { inline static ptrdiff_t stringify(udSlice<char> buffer, udString format, const void *pData) { return udStringify(buffer, format, (int64_t)*(int16_t*)pData); } };
  template<> struct VarArg::StringifyProxy<uint32_t> { inline static ptrdiff_t stringify(udSlice<char> buffer, udString format, const void *pData) { return udStringify(buffer, format, (uint64_t)*(uint32_t*)pData); } };
  template<> struct VarArg::StringifyProxy<int32_t>  { inline static ptrdiff_t stringify(udSlice<char> buffer, udString format, const void *pData) { return udStringify(buffer, format, (int64_t)*(int32_t*)pData); } };
  template<> struct VarArg::StringifyProxy<float>    { inline static ptrdiff_t stringify(udSlice<char> buffer, udString format, const void *pData) { return udStringify(buffer, format, (double)*(float*)pData); } };
  template<size_t N>
  struct VarArg::StringifyProxy<const char[N]>       { inline static ptrdiff_t stringify(udSlice<char> buffer, udString format, const void *pData) { return udStringify(buffer, format, udString((const char*)pData, N-1)); } };

  size_t getLength(udSlice<VarArg> args);
  udSlice<char> concatenate(udSlice<char> buffer, udSlice<VarArg> args);
  udSlice<char> format(udString format, udSlice<char> buffer, udSlice<VarArg> args);
}


// udMutableString

template<size_t Size>
template<typename... Args>
inline udMutableString<Size>& udMutableString<Size>::concat(const Args&... args)
{
  using namespace ud_internal;
  clear();
  VarArg proxies[] = { VarArg(args)... };
  appendInternal(udSlice<VarArg>(proxies, UDARRAYSIZE(proxies)));
  return *this;
}
template<size_t Size>
template<typename... Args>
inline udMutableString<Size>& udMutableString<Size>::append(const Args&... args)
{
  using namespace ud_internal;
  VarArg proxies[] = { VarArg(args)... };
  appendInternal(udSlice<VarArg>(proxies, UDARRAYSIZE(proxies)));
  return *this;
}
template<size_t Size>
template<typename... Args>
inline udMutableString<Size>& udMutableString<Size>::format(udString format, const Args&... args)
{
  using namespace ud_internal;
  VarArg proxies[] = { VarArg(args)... };
  formatInternal(format, udSlice<VarArg>(proxies, UDARRAYSIZE(proxies)));
  return *this;
}

template<size_t Size>
inline void udMutableString<Size>::appendInternal(udSlice<ud_internal::VarArg> args)
{
  using namespace ud_internal;
  size_t len = getLength(args);
  reserve(length + len + 1);
  concatenate(udSlice<char>(ptr + length, len), args);
  length += len;
  ptr[length] = 0;
}
template<size_t Size>
inline void udMutableString<Size>::formatInternal(udString format, udSlice<ud_internal::VarArg> args)
{
  using namespace ud_internal;
  size_t len = ud_internal::format(format, nullptr, args).length;
  reserve(len + 1);
  ud_internal::format(format, udSlice<char>(ptr, len), args);
  length = len;
  ptr[len] = 0;
}


// udSharedString

template<typename... Args>
inline udSharedString udSharedString::concat(const Args&... args)
{
  using namespace ud_internal;
  VarArg proxies[] = { VarArg(args)... };
  return concatInternal(udSlice<VarArg>(proxies, UDARRAYSIZE(proxies)));
}
template<typename... Args>
inline udSharedString udSharedString::format(udString format, const Args&... args)
{
  using namespace ud_internal;
  VarArg proxies[] = { VarArg(args)... };
  return formatInternal(format, udSlice<VarArg>(proxies, UDARRAYSIZE(proxies)));
}
