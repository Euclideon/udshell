#include "ep/eputf.h"

template<typename T>
ptrdiff_t epStringifyTemplate(Slice<char> buffer, String format, const T &val, const epVarArg *pArgs);

ptrdiff_t epStringify(Slice<char> buffer, String format, nullptr_t, const epVarArg *pArgs);
template<typename C>
ptrdiff_t epStringify(Slice<char> buffer, String format, BaseString<C> s, const epVarArg *pArgs);
ptrdiff_t epStringify(Slice<char> buffer, String format, bool b, const epVarArg *pArgs);
ptrdiff_t epStringify(Slice<char> buffer, String format, int64_t i, const epVarArg *pArgs);
ptrdiff_t epStringify(Slice<char> buffer, String format, uint64_t i, const epVarArg *pArgs);
ptrdiff_t epStringify(Slice<char> buffer, String format, double i, const epVarArg *pArgs);
template<typename T>
ptrdiff_t epStringify(Slice<char> buffer, String format, Slice<T> arr, const epVarArg *pArgs)
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

namespace ep {

template<typename C>
class CString
{
  friend struct BaseString<C>;
public:
  operator const C*() const { return pCStr; }
  ~CString()
  {
    epFree((void*)pCStr);
  }

private:
  const C *pCStr;
  CString(BaseString<C> str)
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

// String
template<typename C>
inline BaseString<C>::BaseString()
{}

template<typename C>
inline BaseString<C>::BaseString(const C *ptr, size_t length)
  : Slice<const C>(ptr, length)
{}

template<typename C>
template<typename C2>
inline BaseString<C>::BaseString(Slice<C2> rh)
  : Slice<const C>(rh)
{}

template<typename C>
inline BaseString<C>::BaseString(const C *pString)
  : Slice<const C>(pString, pString ? epStrlen(pString) : 0)
{}

template<typename C>
template<size_t N>
inline BaseString<C>::BaseString(const C str[N])
  : Slice<const C>(str, N)
{}

template<typename C>
BaseString<C>::BaseString(epString s)
  : Slice<const C>((const C*)s.ptr, s.length)
{
  EPASSERT(sizeof(C) == sizeof(char), "Wrong type!");
}
template<typename C>
BaseString<C>::operator epString() const
{
  EPASSERT(sizeof(C) == sizeof(char), "Wrong type!");
  epString s;
  s.length = this->length;
  s.ptr = this->ptr;
  return s;
}

template<typename C>
inline BaseString<C>& BaseString<C>::operator =(Slice<const C> rh)
{
  this->length = rh.length;
  this->ptr = rh.ptr;
  return *this;
}

template<typename C>
inline BaseString<C>& BaseString<C>::operator =(const C *pString)
{
  this->ptr = pString;
  this->length = pString ? epStrlen(pString) : (size_t)0;
  return *this;
}

template<typename C>
inline BaseString<C> BaseString<C>::slice(ptrdiff_t first, ptrdiff_t last) const
{
  size_t start = (size_t)(first < 0 ? first + this->length : first);
  size_t end = (size_t)(last < 0 ? last + this->length : last);
  EPASSERT(end <= this->length && start <= end, "Index out of range!");
  return BaseString<C>(this->ptr + start, end - start);
}

template<typename C>
inline size_t BaseString<C>::numChars() const
{
  BaseString<C> t = *this;
  size_t numChars = 0;
  while (t.length)
  {
    t.popFrontChar();
    ++numChars;
  }
  return numChars;
}
template<>
inline size_t BaseString<char32_t>::numChars() const
{
  return length;
}

template<typename C>
inline char32_t BaseString<C>::frontChar() const
{
  char32_t r;
  epUTFDecode(this->ptr, &r);
  return r;
}
template<>
inline char32_t BaseString<char>::backChar() const
{
  const char *pLast = ptr + length-1;
  while ((*pLast & 0xC) == 0x80)
    --pLast;
  char32_t r;
  epUTFDecode(pLast, &r);
  return r;
}
template<>
inline char32_t BaseString<char16_t>::backChar() const
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
inline char32_t BaseString<char32_t>::backChar() const
{
  return ptr[length-1];
}

template<typename C>
inline char32_t BaseString<C>::popFrontChar()
{
  char32_t r;
  size_t codeUnits = epUTFDecode(this->ptr, &r);
  this->ptr += codeUnits;
  this->length -= codeUnits;
  return r;
}
template<>
inline char32_t BaseString<char>::popBackChar()
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
inline char32_t BaseString<char16_t>::popBackChar()
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
inline char32_t BaseString<char32_t>::popBackChar()
{
  return ptr[--length];
}

template<typename C>
inline bool BaseString<C>::eq(BaseString<C> rh) const
{
  return Slice<const C>::eq(rh);
}
template<typename C>
inline bool BaseString<C>::eqIC(BaseString<C> rh) const
{
  if (this->length != rh.length)
    return false;
  for (size_t i = 0; i<this->length; ++i)
    if (toLower(this->ptr[i]) != toLower(rh.ptr[i]))
      return false;
  return true;
}
template<typename C>
inline bool BaseString<C>::beginsWith(BaseString<C> rh) const
{
  return Slice<const C>::beginsWith(rh);
}
template<typename C>
inline bool BaseString<C>::beginsWithIC(BaseString<C> rh) const
{
  if (this->length < rh.length)
    return false;
  return slice(0, rh.length).eqIC(rh);
}
template<typename C>
inline bool BaseString<C>::endsWith(BaseString<C> rh) const
{
  return Slice<const C>::endsWith(rh);
}
template<typename C>
inline bool BaseString<C>::endsWithIC(BaseString<C> rh) const
{
  if (this->length < rh.length)
    return false;
  return slice(this->length - rh.length, this->length).eqIC(rh);
}

template<typename C>
inline ptrdiff_t BaseString<C>::cmp(BaseString<C> rh) const
{
  return Slice<const C>::cmp(rh);
}
template<typename C>
inline ptrdiff_t BaseString<C>::cmpIC(BaseString<C> rh) const
{
  size_t len = this->length < rh.length ? this->length : rh.length;
  for (size_t i = 0; i < len; ++i)
  {
    char a = toLower(this->ptr[i]), b = toLower(rh.ptr[i]);
    if (a == b)
      continue;
    return a < b ? -1 : 1;
  }
  return this->length - rh.length;
}

template<typename C>
inline C* BaseString<C>::toStringz(C *pBuffer, size_t bufferLen) const
{
  size_t len = this->length < bufferLen-1 ? this->length : bufferLen-1;
  memcpy(pBuffer, this->ptr, len);
  pBuffer[len] = 0;
  return pBuffer;
}

template<typename C>
inline CString<C> BaseString<C>::toStringz() const
{
  return CString<C>(*this);
}

template<typename C>
inline size_t BaseString<C>::findFirstIC(BaseString<C> s) const
{
  if (s.empty())
    return 0;
  ptrdiff_t len = this->length-s.length;
  for (ptrdiff_t i = 0; i < len; ++i)
  {
    size_t j = 0;
    for (; j < s.length; ++j)
    {
      if (toLower(this->ptr[i + j]) != toLower(s.ptr[j]))
        break;
    }
    if (j == s.length)
      return i;
  }
  return this->length;
}
template<typename C>
inline size_t BaseString<C>::findLastIC(BaseString<C> s) const
{
  if (s.empty())
    return this->length;
  for (ptrdiff_t i = this->length-s.length; i >= 0; --i)
  {
    size_t j = 0;
    for (; j < s.length; ++j)
    {
      if (toLower(this->ptr[i + j]) != toLower(s.ptr[j]))
        break;
    }
    if (j == s.length)
      return i;
  }
  return this->length;
}

template<typename C>
inline BaseString<C> BaseString<C>::getLeftAtFirstIC(BaseString<C> s, bool bInclusive) const
{
  return slice(0, findFirstIC(s) + (bInclusive ? s.length : 0));
}
template<typename C>
inline BaseString<C> BaseString<C>::getLeftAtLastIC(BaseString<C> s, bool bInclusive) const
{
  return slice(0, findLastIC(s) + (bInclusive ? s.length : 0));
}
template<typename C>
inline BaseString<C> BaseString<C>::getRightAtFirstIC(BaseString<C> s, bool bInclusive) const
{
  return slice(findFirstIC(s) + (bInclusive ? 0 : s.length), this->length);
}
template<typename C>
inline BaseString<C> BaseString<C>::getRightAtLastIC(BaseString<C> s, bool bInclusive) const
{
  return slice(findLastIC(s) + (bInclusive ? 0 : s.length), this->length);
}

template<typename C>
template<bool Front, bool Back>
inline BaseString<C> BaseString<C>::trim() const
{
  size_t first = 0, last = this->length;
  if (Front)
  {
    while (epIsWhitespace(this->ptr[first]) && first < this->length)
      ++first;
  }
  if (Back)
  {
    while (last > first && epIsWhitespace(this->ptr[last - 1]))
      --last;
  }
  return BaseString<C>(this->ptr + first, last - first);
}

template<typename C>
template<bool skipEmptyTokens>
inline BaseString<C> BaseString<C>::popToken(BaseString<C> delimiters)
{
  return Slice<const C>::template popToken<skipEmptyTokens>(delimiters);
}

template<typename C>
template<bool skipEmptyTokens>
inline Slice<BaseString<C>> BaseString<C>::tokenise(Slice<BaseString<C>> tokens, BaseString<C> delimiters)
{
  return Slice<const C>::tokenise<skipEmptyTokens>(tokens, delimiters);
}

template<typename C>
inline uint32_t BaseString<C>::hash(uint32_t hash) const
{
  size_t i = 0;
  while (i < this->length)
  {
    hash ^= (uint32_t)this->ptr[i++];
    hash *= 0x01000193;
  }
  return hash;
}


// MutableString
template<size_t Size>
inline MutableString<Size>::MutableString()
{}

template<size_t Size>
inline MutableString<Size>::MutableString(const MutableString<Size> &rh)
  : Array<char, Size>(rh)
{}
template<size_t Size>
inline MutableString<Size>::MutableString(MutableString<Size> &&rval)
  : Array<char, Size>(std::move(rval))
{}

template<size_t Size>
template <size_t Len>
inline MutableString<Size>::MutableString(Array<char, Len> &&rval)
  : Array<char, Size>(std::move(rval))
{}

template<size_t Size>
template <typename U>
inline MutableString<Size>::MutableString(U *ptr, size_t length)
  : Array<char, Size>(ptr, length)
{}

template<size_t Size>
template <typename U>
inline MutableString<Size>::MutableString(Slice<U> slice)
  : Array<char, Size>(slice)
{}

template<size_t Size>
inline MutableString<Size>::MutableString(const char *pString)
  : Array<char, Size>(pString, pString ? epStrlen(pString) : 0)
{}

template<size_t Size>
inline MutableString<Size>& MutableString<Size>::operator =(const MutableString<Size> &rh)
{
  Array<char, Size>::operator=(rh);
  return *this;
}
template<size_t Size>
inline MutableString<Size>& MutableString<Size>::operator =(MutableString<Size> &&rval)
{
  Array<char, Size>::operator=(std::move(rval));
  return *this;
}
template<size_t Size>
template <size_t Len>
inline MutableString<Size>& MutableString<Size>::operator =(Array<char, Len> &&rval)
{
  Array<char, Size>::operator=(std::move(rval));
  return *this;
}
template<size_t Size>
template <typename U>
inline MutableString<Size>& MutableString<Size>::operator =(Slice<U> rh)
{
  Array<char, Size>::operator=(rh);
  return *this;
}
template<size_t Size>
inline MutableString<Size>& MutableString<Size>::operator =(const char *pString)
{
  Array<char, Size>::operator=(String(pString, pString ? epStrlen(pString) : 0));
  return *this;
}

template<size_t Size>
inline MutableString<Size>& MutableString<Size>::sprintf(const char *pFormat, ...)
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
inline MutableString<Size>& MutableString<Size>::toUpper()
{
  for (size_t i = 0; i < this->length; ++i)
  {
    if (isAlpha(this->ptr[i]))
      this->ptr[i] == toUpper(this->ptr[i]);
  }
}
template<size_t Size>
inline MutableString<Size>& MutableString<Size>::toLower()
{
  for (size_t i = 0; i < this->length; ++i)
  {
    if (isAlpha(this->ptr[i]))
      this->ptr[i] == toLower(this->ptr[i]);
  }
}

template<size_t Size>
inline CString<char> MutableString<Size>::toStringz() const
{
  return ((BaseString<char>*)this)->toStringz();
}


// SharedString
inline SharedString::SharedString()
{}

inline SharedString::SharedString(const SharedString &val)
  : SharedArray<const char>(val)
{}
inline SharedString::SharedString(SharedString &&rval)
  : SharedArray<const char>(std::move(rval))
{}

inline SharedString::SharedString(const SharedArray<const char> &rcstr)
  : SharedArray<const char>(rcstr)
{}
inline SharedString::SharedString(SharedArray<const char> &&rval)
  : SharedArray<const char>(std::move(rval))
{}

template <typename U, size_t Len>
inline SharedString::SharedString(const Array<U, Len> &arr)
  : SharedArray<const char>(arr)
{}
template <typename U, size_t Len>
inline SharedString::SharedString(Array<U, Len> &&rval)
  : SharedArray<const char>(std::move(rval))
{}

template <typename U>
inline SharedString::SharedString(U *ptr, size_t length)
  : SharedArray<const char>(ptr, length)
{}
template <typename U>
inline SharedString::SharedString(Slice<U> slice)
  : SharedArray<const char>(slice)
{}

inline SharedString::SharedString(const char *pString)
  : SharedArray<const char>(pString, pString ? epStrlen(pString) : 0)
{}

inline SharedString::SharedString(epSharedString s)
  : SharedString((SharedString&)s)
{}
inline SharedString::operator epSharedString() const
{
  epSharedString s;
  s.length = this->length;
  s.ptr = this->ptr;
  return s;
}

inline SharedString& SharedString::operator =(const SharedString &val)
{
  SharedArray<const char>::operator=(val);
  return *this;
}
inline SharedString& SharedString::operator =(SharedString &&rval)
{
  SharedArray<const char>::operator=(std::move(rval));
  return *this;
}

inline SharedString& SharedString::operator =(const SharedArray<const char> &rh)
{
  SharedArray<const char>::operator=(rh);
  return *this;
}
inline SharedString& SharedString::operator =(SharedArray<const char> &&rval)
{
  SharedArray<const char>::operator=(std::move(rval));
  return *this;
}
template <typename U>
inline SharedString& SharedString::operator =(Slice<U> rh)
{
  SharedArray<const char>::operator=(rh);
  return *this;
}
inline SharedString& SharedString::operator =(const char *pString)
{
  *this = SharedString(pString);
  return *this;
}

inline CString<char> SharedString::toStringz() const
{
  return ((String*)this)->toStringz();
}

inline SharedString SharedString::asUpper() const
{
  EPASSERT(false, "TODO");
  return nullptr;
}
inline SharedString SharedString::asLower() const
{
  EPASSERT(false, "TODO");
  return nullptr;
}


//
// varargs functions....
//

// stringify helper
namespace internal {

  template<typename T>
  struct StringifyProxy
  {
    static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs)
    {
      using ::epStringify;
      return epStringify(buffer, format, *(T*)pData, pArgs);
    }
    static const size_t intify = 0;
  };

  // make the numeric types promote explicitly
  template<> struct StringifyProxy<bool>     { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, *(bool*)pData, pArgs); }               inline static int64_t intify(const void *pData) { return *(bool*)pData ? 1 : 0; } };
  template<> struct StringifyProxy<uint8_t>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, (uint64_t)*(uint8_t*)pData, pArgs); }  inline static int64_t intify(const void *pData) { return (int64_t)*(uint8_t*)pData; } };
  template<> struct StringifyProxy<int8_t>   { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, (int64_t)*(int8_t*)pData, pArgs); }    inline static int64_t intify(const void *pData) { return (int64_t)*(int8_t*)pData; } };
  template<> struct StringifyProxy<uint16_t> { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, (uint64_t)*(uint16_t*)pData, pArgs); } inline static int64_t intify(const void *pData) { return (int64_t)*(uint16_t*)pData; } };
  template<> struct StringifyProxy<int16_t>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, (int64_t)*(int16_t*)pData, pArgs); }   inline static int64_t intify(const void *pData) { return (int64_t)*(int16_t*)pData; } };
  template<> struct StringifyProxy<uint32_t> { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, (uint64_t)*(uint32_t*)pData, pArgs); } inline static int64_t intify(const void *pData) { return (int64_t)*(uint32_t*)pData; } };
  template<> struct StringifyProxy<int32_t>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, (int64_t)*(int32_t*)pData, pArgs); }   inline static int64_t intify(const void *pData) { return (int64_t)*(int32_t*)pData; } };
  template<> struct StringifyProxy<uint64_t> { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, *(uint64_t*)pData, pArgs); }           inline static int64_t intify(const void *pData) { return (int64_t)*(uint64_t*)pData; } };
  template<> struct StringifyProxy<int64_t>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, *(int64_t*)pData, pArgs); }            inline static int64_t intify(const void *pData) { return *(int64_t*)pData; } };
  template<> struct StringifyProxy<float>    { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, (double)*(float*)pData, pArgs); }      inline static int64_t intify(const void *pData) { return (int64_t)*(float*)pData; } };
  template<> struct StringifyProxy<double>   { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, *(double*)pData, pArgs); }             inline static int64_t intify(const void *pData) { return (int64_t)*(double*)pData; } };

  template<> struct StringifyProxy<char*>           { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const char *pS = *(char**)pData;               return ::epStringify(buffer, format, String(pS, pS ? epStrlen(pS) : 0), pArgs); }  static const size_t intify = 0; };
  template<> struct StringifyProxy<char16_t*>       { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const char16_t *pS = *(char16_t**)pData;       return ::epStringify(buffer, format, WString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
  template<> struct StringifyProxy<char32_t*>       { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const char32_t *pS = *(char32_t**)pData;       return ::epStringify(buffer, format, DString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
  template<> struct StringifyProxy<const char*>     { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const char *pS = *(const char**)pData;         return ::epStringify(buffer, format, String(pS, pS ? epStrlen(pS) : 0), pArgs); }  static const size_t intify = 0; };
  template<> struct StringifyProxy<const char16_t*> { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const char16_t *pS = *(const char16_t**)pData; return ::epStringify(buffer, format, WString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
  template<> struct StringifyProxy<const char32_t*> { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const char32_t *pS = *(const char32_t**)pData; return ::epStringify(buffer, format, DString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
  template<size_t N>
  struct StringifyProxy<char[N]>                  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const char *pS = (const char*)pData;         return ::epStringify(buffer, format, String(pS, N-1), pArgs); }  static const size_t intify = 0; };
  template<size_t N>
  struct StringifyProxy<char16_t[N]>              { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const char16_t *pS = (const char16_t*)pData; return ::epStringify(buffer, format, WString(pS, N-1), pArgs); } static const size_t intify = 0; };
  template<size_t N>
  struct StringifyProxy<char32_t[N]>              { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const char32_t *pS = (const char32_t*)pData; return ::epStringify(buffer, format, DString(pS, N-1), pArgs); } static const size_t intify = 0; };
  template<size_t N>
  struct StringifyProxy<MutableString<N>>         { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, *(String*)pData, pArgs); } static const size_t intify = 0; };
  template<> struct StringifyProxy<SharedString>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return ::epStringify(buffer, format, *(String*)pData, pArgs); } static const size_t intify = 0; };

  size_t getLength(Slice<epVarArg> args);
  Slice<char> concatenate(Slice<char> buffer, Slice<epVarArg> args);
  Slice<char> format(String format, Slice<char> buffer, Slice<epVarArg> args);

} // namespace internal

template<typename T>
epVarArg::epVarArg(const T& arg)
  : pArg(&arg)
  , pStringProxy(internal::StringifyProxy<T>::stringify)
  , pIntProxy((IntConvFunc*)internal::StringifyProxy<T>::intify)
{}


// MutableString

template<size_t Size>
template<typename... Args>
inline MutableString<Size>& MutableString<Size>::concat(const Args&... args)
{
  this->clear();
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  appendInternal(Slice<epVarArg>(proxies, sizeof...(Args)));
  return *this;
}
template<size_t Size>
template<typename... Args>
inline MutableString<Size>& MutableString<Size>::append(const Args&... args)
{
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  appendInternal(Slice<epVarArg>(proxies, sizeof...(Args)));
  return *this;
}
template<size_t Size>
template<typename... Args>
inline MutableString<Size>& MutableString<Size>::format(String format, const Args&... args)
{
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  formatInternal(format, Slice<epVarArg>(proxies, sizeof...(Args)));
  return *this;
}

template<size_t Size>
inline void MutableString<Size>::appendInternal(Slice<epVarArg> args)
{
  size_t len = internal::getLength(args);
  this->reserve(this->length + len + 1);
  internal::concatenate(Slice<char>(this->ptr + this->length, len), args);
  this->length += len;
  this->ptr[this->length] = 0;
}
template<size_t Size>
inline void MutableString<Size>::formatInternal(String format, Slice<epVarArg> args)
{
  size_t len = internal::format(format, nullptr, args).length;
  this->reserve(len + 1);
  internal::format(format, Slice<char>(this->ptr, len), args);
  this->length = len;
  this->ptr[len] = 0;
}


// SharedString

template<typename... Args>
inline SharedString SharedString::concat(const Args&... args)
{
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  return concatInternal(Slice<epVarArg>(proxies, sizeof...(Args)));
}
template<typename... Args>
inline SharedString SharedString::format(String format, const Args&... args)
{
  epVarArg proxies[sizeof...(Args)+1] = { epVarArg(args)... };
  return formatInternal(format, Slice<epVarArg>(proxies, sizeof...(Args)));
}

} // namespace ep

template<typename T>
epforceinline ptrdiff_t epStringifyTemplate(Slice<char> buffer, String format, const T &val, const epVarArg *pArgs)
{
  return internal::StringifyProxy<T>::stringify(buffer, format, (void*)&val, pArgs);
}
