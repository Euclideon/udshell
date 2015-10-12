
#include "eputf.h"

// TODO: remove these?
#include <string.h>
#include <stdarg.h>

#include <utility>


template<typename C>
class epCString
{
  friend struct epBaseString<C>;
public:
  operator const C*() const { return pCStr; }
  ~epCString()
  {
    epFree((void*)pCStr);
  }

private:
  const C *pCStr;
  epCString(epBaseString<C> str)
  {
    C *buf = (C*)epAlloc((str.length + 1) * sizeof(C));
    memcpy(buf, str.ptr, str.length*sizeof(C));
    buf[str.length] = 0;
    pCStr = buf;
  }
};

inline size_t epStrlen(const char *pStr) { return strlen(pStr); }
inline size_t epStrlen(const char16_t *pStr)
{
  const char16_t *pEnd = pStr;
  while (*pEnd != 0)
    ++pEnd;
  return pEnd - pStr;
}
inline size_t epStrlen(const char32_t *pStr)
{
  const char32_t *pEnd = pStr;
  while (*pEnd != 0)
    ++pEnd;
  return pEnd - pStr;
}

inline size_t epUTF8Len(const char *pStr)
{
  return strlen(pStr);
}
inline size_t epUTF8Len(const char16_t *pStr)
{
  size_t len = 0;
  while (*pStr)
  {
    size_t srcLen;
    len += epUTF8SequenceLength(pStr, &srcLen);
    pStr += srcLen;
  }
  return len;
}
inline size_t epUTF8Len(const char32_t *pStr)
{
  size_t len = 0;
  while (*pStr)
  {
    len += epUTF8SequenceLength(pStr);
    ++pStr;
  }
  return len;
}

// epString
template<typename C>
inline epBaseString<C>::epBaseString()
{}

template<typename C>
inline epBaseString<C>::epBaseString(const C *ptr, size_t length)
  : epSlice<const C>(ptr, length)
{}

template<typename C>
template<typename C2>
inline epBaseString<C>::epBaseString(epSlice<C2> rh)
  : epSlice<const C>(rh)
{}

template<typename C>
inline epBaseString<C>::epBaseString(const C *pString)
  : epSlice<const C>(pString, pString ? epStrlen(pString) : 0)
{}

template<typename C>
template<size_t N>
inline epBaseString<C>::epBaseString(const C str[N])
  : epSlice<const C>(str, N)
{}

template<typename C>
inline epBaseString<C>& epBaseString<C>::operator =(epSlice<const C> rh)
{
  length = rh.length;
  ptr = rh.ptr;
  return *this;
}

template<typename C>
inline epBaseString<C>& epBaseString<C>::operator =(const C *pString)
{
  ptr = pString;
  length = pString ? epStrlen(pString) : (size_t)0;
  return *this;
}

template<typename C>
inline epBaseString<C> epBaseString<C>::slice(ptrdiff_t first, ptrdiff_t last) const
{
  size_t start = (size_t)(first < 0 ? first + length : first);
  size_t end = (size_t)(last < 0 ? last + length : last);
  EPASSERT(end <= length && start <= end, "Index out of range!");
  return epBaseString<C>(ptr + start, end - start);
}

template<typename C>
inline size_t epBaseString<C>::numChars() const
{
  epBaseString<C> t = *this;
  size_t numChars = 0;
  while (t.length)
  {
    t.popFrontChar();
    ++numChars;
  }
  return numChars;
}
template<>
inline size_t epBaseString<char32_t>::numChars() const
{
  return length;
}

template<typename C>
inline char32_t epBaseString<C>::frontChar() const
{
  char32_t r;
  epUTFDecode(ptr, &r);
  return r;
}
template<>
inline char32_t epBaseString<char>::backChar() const
{
  const char *pLast = ptr + length-1;
  while ((*pLast & 0xC) == 0x80)
    --pLast;
  char32_t r;
  epUTFDecode(pLast, &r);
  return r;
}
template<>
inline char32_t epBaseString<char16_t>::backChar() const
{
  char16_t back = ptr[length-1];
  if (back >= 0xD800 && (back & 0xFC00) == 0xDC00)
  {
    char32_t r;
    epUTFDecode(ptr + length-2, &r);
    return r;
  }
  return back;
}
template<>
inline char32_t epBaseString<char32_t>::backChar() const
{
  return ptr[length-1];
}

template<typename C>
inline char32_t epBaseString<C>::popFrontChar()
{
  char32_t r;
  size_t codeUnits = epUTFDecode(ptr, &r);
  ptr += codeUnits;
  length -= codeUnits;
  return r;
}
template<>
inline char32_t epBaseString<char>::popBackChar()
{
  size_t numChars = 1;
  while ((ptr[length - numChars] & 0xC) == 0x80)
    --numChars;
  char32_t r;
  epUTFDecode(ptr + length - numChars, &r);
  length -= numChars;
  return r;
}
template<>
inline char32_t epBaseString<char16_t>::popBackChar()
{
  char16_t back = ptr[--length];
  if (back >= 0xD800 && (back & 0xFC00) == 0xDC00)
  {
    char32_t r;
    epUTFDecode(ptr + --length, &r);
    return r;
  }
  return back;
}
template<>
inline char32_t epBaseString<char32_t>::popBackChar()
{
  return ptr[--length];
}

template<typename C>
inline bool epBaseString<C>::eq(epBaseString<C> rh) const
{
  return epSlice<const C>::eq(rh);
}
template<typename C>
inline bool epBaseString<C>::eqIC(epBaseString<C> rh) const
{
  if(length != rh.length)
    return false;
  for (size_t i = 0; i<length; ++i)
    if (toLower(ptr[i]) != toLower(rh.ptr[i]))
      return false;
  return true;
}
template<typename C>
inline bool epBaseString<C>::beginsWith(epBaseString<C> rh) const
{
  return epSlice<const C>::beginsWith(rh);
}
template<typename C>
inline bool epBaseString<C>::beginsWithIC(epBaseString<C> rh) const
{
  if (length < rh.length)
    return false;
  return slice(0, rh.length).eqIC(rh);
}
template<typename C>
inline bool epBaseString<C>::endsWith(epBaseString<C> rh) const
{
  return epSlice<const C>::endsWith(rh);
}
template<typename C>
inline bool epBaseString<C>::endsWithIC(epBaseString<C> rh) const
{
  if (length < rh.length)
    return false;
  return slice(length - rh.length, length).eqIC(rh);
}

template<typename C>
inline ptrdiff_t epBaseString<C>::cmp(epBaseString<C> rh) const
{
  return epSlice<const C>::cmp(rh);
}
template<typename C>
inline ptrdiff_t epBaseString<C>::cmpIC(epBaseString<C> rh) const
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

template<typename C>
inline C* epBaseString<C>::toStringz(C *pBuffer, size_t bufferLen) const
{
  size_t len = length < bufferLen-1 ? length : bufferLen-1;
  memcpy(pBuffer, ptr, len);
  pBuffer[len] = 0;
  return pBuffer;
}

template<typename C>
inline epCString<C> epBaseString<C>::toStringz() const
{
  return epCString<C>(*this);
}

template<typename C>
inline size_t epBaseString<C>::findFirstIC(epBaseString<C> s) const
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
template<typename C>
inline size_t epBaseString<C>::findLastIC(epBaseString<C> s) const
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

template<typename C>
inline epBaseString<C> epBaseString<C>::getLeftAtFirstIC(epBaseString<C> s, bool bInclusive) const
{
  return slice(0, findFirstIC(s) + (bInclusive ? s.length : 0));
}
template<typename C>
inline epBaseString<C> epBaseString<C>::getLeftAtLastIC(epBaseString<C> s, bool bInclusive) const
{
  return slice(0, findLastIC(s) + (bInclusive ? s.length : 0));
}
template<typename C>
inline epBaseString<C> epBaseString<C>::getRightAtFirstIC(epBaseString<C> s, bool bInclusive) const
{
  return slice(findFirstIC(s) + (bInclusive ? 0 : s.length), length);
}
template<typename C>
inline epBaseString<C> epBaseString<C>::getRightAtLastIC(epBaseString<C> s, bool bInclusive) const
{
  return slice(findLastIC(s) + (bInclusive ? 0 : s.length), length);
}

template<typename C>
template<bool Front, bool Back>
inline epBaseString<C> epBaseString<C>::trim() const
{
  size_t first = 0, last = length;
  if (Front)
  {
    while (isWhitespace(ptr[first]) && first < length)
      ++first;
  }
  if (Back)
  {
    while (last > first && isWhitespace(ptr[last - 1]))
      --last;
  }
  return epBaseString<C>(ptr + first, last - first);
}

template<typename C>
template<bool skipEmptyTokens>
inline epBaseString<C> epBaseString<C>::popToken(epBaseString<C> delimiters)
{
  return epSlice<const C>::popToken<skipEmptyTokens>(delimiters);
}

template<typename C>
template<bool skipEmptyTokens>
inline epSlice<epBaseString<C>> epBaseString<C>::tokenise(epSlice<epBaseString<C>> tokens, epBaseString<C> delimiters)
{
  return epSlice<const C>::tokenise<skipEmptyTokens>(tokens, delimiters);
}

template<typename C>
inline uint32_t epBaseString<C>::hash(uint32_t hash) const
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
  : epArray<char, Size>(pString, pString ? epStrlen(pString) : 0)
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
  epArray<char, Size>::operator=(epString(pString, pString ? epStrlen(pString) : 0));
  return *this;
}

template<size_t Size>
inline epMutableString<Size>& epMutableString<Size>::sprintf(const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);

#if defined(EP_COMPILER_VISUALC)
  size_t len = _vscprintf(pFormat, args) + 1;
#else
  size_t len = vsprintf(nullptr, pFormat, args) + 1;
#endif

  this->reserve(len + 1);

#if defined(EP_COMPILER_VISUALC)
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
inline epCString<char> epMutableString<Size>::toStringz() const
{
  return ((epBaseString<char>*)this)->toStringz();
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
  : epSharedSlice<const char>(pString, pString ? epStrlen(pString) : 0)
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

inline epCString<char> epSharedString::toStringz() const
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

template<bool Front, bool Back>
inline epSharedString epSharedString::trim() const
{
  epString s = ((epString*)this)->trim<Front, Back>();
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
template<typename C>
ptrdiff_t epStringify(epSlice<char> buffer, epString format, epBaseString<C> s, const epVarArg *pArgs);
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
namespace ep {
namespace internal {

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
template<> struct StringifyProxy<bool>     { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, *(bool*)pData, pArgs); }               inline static int64_t intify(const void *pData) { return *(bool*)pData ? 1 : 0; } };
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

template<> struct StringifyProxy<char*>           { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { const char *pS = *(char**)pData;               return epStringify(buffer, format, epString(pS, pS ? epStrlen(pS) : 0), pArgs); }  static const size_t intify = 0; };
template<> struct StringifyProxy<char16_t*>       { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { const char16_t *pS = *(char16_t**)pData;       return epStringify(buffer, format, epWString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<char32_t*>       { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { const char32_t *pS = *(char32_t**)pData;       return epStringify(buffer, format, epDString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<const char*>     { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { const char *pS = *(const char**)pData;         return epStringify(buffer, format, epString(pS, pS ? epStrlen(pS) : 0), pArgs); }  static const size_t intify = 0; };
template<> struct StringifyProxy<const char16_t*> { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { const char16_t *pS = *(const char16_t**)pData; return epStringify(buffer, format, epWString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<const char32_t*> { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { const char32_t *pS = *(const char32_t**)pData; return epStringify(buffer, format, epDString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
template<size_t N>
struct StringifyProxy<const char[N]>              { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { const char *pS = (const char*)pData;         return epStringify(buffer, format, epString(pS, N-1), pArgs); }  static const size_t intify = 0; };
template<size_t N>
struct StringifyProxy<const char16_t[N]>          { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { const char16_t *pS = (const char16_t*)pData; return epStringify(buffer, format, epWString(pS, N-1), pArgs); } static const size_t intify = 0; };
template<size_t N>
struct StringifyProxy<const char32_t[N]>          { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { const char32_t *pS = (const char32_t*)pData; return epStringify(buffer, format, epDString(pS, N-1), pArgs); } static const size_t intify = 0; };
template<size_t N>
struct StringifyProxy<epMutableString<N>>         { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, *(epString*)pData, pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<epSharedString>  { inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringify(buffer, format, *(epString*)pData, pArgs); } static const size_t intify = 0; };

size_t getLength(epSlice<epVarArg> args);
epSlice<char> concatenate(epSlice<char> buffer, epSlice<epVarArg> args);
epSlice<char> format(epString format, epSlice<char> buffer, epSlice<epVarArg> args);

} // namespace internal
} // namespace ep

template<typename T>
epVarArg::epVarArg(const T& arg)
: pArg(&arg)
, pStringProxy(ep::internal::StringifyProxy<T>::stringify)
, pIntProxy((IntConvFunc*)ep::internal::StringifyProxy<T>::intify)
{}


template<typename T>
inline ptrdiff_t epStringifyTemplate(epSlice<char> buffer, epString format, const T &val, const epVarArg *pArgs)
{
  return ep::internal::StringifyProxy<T>::stringify(buffer, format, (void*)&val, pArgs);
}


// epMutableString

template<size_t Size>
template<typename... Args>
inline epMutableString<Size>& epMutableString<Size>::concat(const Args&... args)
{
  using namespace ep::internal;
  this->clear();
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  appendInternal(epSlice<epVarArg>(proxies, sizeof...(Args)));
  return *this;
}
template<size_t Size>
template<typename... Args>
inline epMutableString<Size>& epMutableString<Size>::append(const Args&... args)
{
  using namespace ep::internal;
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  appendInternal(epSlice<epVarArg>(proxies, sizeof...(Args)));
  return *this;
}
template<size_t Size>
template<typename... Args>
inline epMutableString<Size>& epMutableString<Size>::format(epString format, const Args&... args)
{
  using namespace ep::internal;
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  formatInternal(format, epSlice<epVarArg>(proxies, sizeof...(Args)));
  return *this;
}

template<size_t Size>
inline void epMutableString<Size>::appendInternal(epSlice<epVarArg> args)
{
  using namespace ep::internal;
  size_t len = getLength(args);
  this->reserve(this->length + len + 1);
  ep::internal::concatenate(epSlice<char>(this->ptr + this->length, len), args);
  this->length += len;
  this->ptr[this->length] = 0;
}
template<size_t Size>
inline void epMutableString<Size>::formatInternal(epString format, epSlice<epVarArg> args)
{
  using namespace ep::internal;
  size_t len = ep::internal::format(format, nullptr, args).length;
  this->reserve(len + 1);
  ep::internal::format(format, epSlice<char>(this->ptr, len), args);
  this->length = len;
  this->ptr[len] = 0;
}


// epSharedString

template<typename... Args>
inline epSharedString epSharedString::concat(const Args&... args)
{
  using namespace ep::internal;
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  return concatInternal(epSlice<epVarArg>(proxies, sizeof...(Args)));
}
template<typename... Args>
inline epSharedString epSharedString::format(epString format, const Args&... args)
{
  using namespace ep::internal;
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  return formatInternal(format, epSlice<epVarArg>(proxies, sizeof...(Args)));
}
