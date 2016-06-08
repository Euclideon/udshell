#include "ep/cpp/utf.h"

template<typename T>
ptrdiff_t epStringifyTemplate(ep::Slice<char> buffer, ep::String format, const T &val, const ep::VarArg *pArgs);

ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, nullptr_t, const ep::VarArg *pArgs);
template<typename C>
ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, ep::BaseString<C> s, const ep::VarArg *pArgs);
inline ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, char c, const ep::VarArg *pArgs)     { return epStringify(buffer, format, ep::BaseString<char>(&c, 1), pArgs); }
inline ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, char16_t c, const ep::VarArg *pArgs) { return epStringify(buffer, format, ep::BaseString<char16_t>(&c, 1), pArgs); }
inline ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, char32_t c, const ep::VarArg *pArgs) { return epStringify(buffer, format, ep::BaseString<char32_t>(&c, 1), pArgs); }
inline ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, wchar_t c, const ep::VarArg *pArgs)  { char32_t d = c; return epStringify(buffer, format, ep::BaseString<char32_t>(&d, 1), pArgs); }
ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, bool b, const ep::VarArg *pArgs);
ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, int64_t i, const ep::VarArg *pArgs);
ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, uint64_t i, const ep::VarArg *pArgs);
ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, double i, const ep::VarArg *pArgs);
template<typename T>
ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, ep::Slice<T> arr, const ep::VarArg *pArgs)
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

namespace internal {
  extern const char s_charDetails[256];

  int epvscprintf(const char * format, va_list args);
  int epvsnprintf(char * s, size_t count, const char * format, va_list args);
}

epforceinline char toLower(char c) { return epIsAlpha(c) ? c|0x20 : c; }
epforceinline char toUpper(char c) { return epIsAlpha(c) ? c&~0x20 : c; }

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

inline size_t UTF8Len(const char *pStr)
{
  return strlen(pStr);
}
inline size_t UTF8Len(const char16_t *pStr)
{
  size_t len = 0;
  while (*pStr)
  {
    size_t srcLen;
    len += UTF8SequenceLength(pStr, &srcLen);
    pStr += srcLen;
  }
  return len;
}
inline size_t UTF8Len(const char32_t *pStr)
{
  size_t len = 0;
  while (*pStr)
  {
    len += UTF8SequenceLength(pStr);
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
inline BaseString<C>::BaseString(const C (&str)[N])
  : Slice<const C>(str, N)
{}

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
  EPASSERT_THROW(end <= this->length && start <= end, Result::OutOfBounds, "Index out of range!");
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
  UTFDecode(this->ptr, &r);
  return r;
}
template<>
inline char32_t BaseString<char>::backChar() const
{
  const char *pLast = ptr + length-1;
  while ((*pLast & 0xC) == 0x80)
    --pLast;
  char32_t r;
  UTFDecode(pLast, &r);
  return r;
}
template<>
inline char32_t BaseString<char16_t>::backChar() const
{
  char16_t back = ptr[length-1];
  if (back >= 0xD800 && (back & 0xFC00) == 0xDC00)
  {
    char32_t r;
    UTFDecode(ptr + length-2, &r);
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
  size_t codeUnits = UTFDecode(this->ptr, &r);
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
  UTFDecode(ptr + length - numChars, &r);
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
    UTFDecode(ptr + --length, &r);
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
  size_t offset = findFirstIC(s);
  if (offset != this->length)
    offset += bInclusive ? s.length : 0;
  return BaseString<C>(this->ptr, offset);
}
template<typename C>
inline BaseString<C> BaseString<C>::getLeftAtLastIC(BaseString<C> s, bool bInclusive) const
{
  size_t offset = findLastIC(s);
  if (offset != this->length)
    offset += bInclusive ? s.length : 0;
  return BaseString<C>(this->ptr, offset);
}
template<typename C>
inline BaseString<C> BaseString<C>::getRightAtFirstIC(BaseString<C> s, bool bInclusive) const
{
  size_t offset = findFirstIC(s);
  if (offset != this->length)
    offset += bInclusive ? 0 : s.length;
  return BaseString<C>(this->ptr + offset, this->length - offset);
}
template<typename C>
inline BaseString<C> BaseString<C>::getRightAtLastIC(BaseString<C> s, bool bInclusive) const
{
  size_t offset = findLastIC(s);
  if (offset != this->length)
    offset += bInclusive ? 0 : s.length;
  return BaseString<C>(this->ptr + offset, this->length - offset);
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
  auto t = Slice<const C>::template tokenise<skipEmptyTokens>(tokens, delimiters);
  return Slice<BaseString<C>>((BaseString<C>*)t.ptr, t.length);
}
template<typename C>
template<bool skipEmptyTokens>
inline size_t BaseString<C>::tokenise(std::function<void(BaseString<C> onToken, size_t index)> onToken, BaseString<C> delimiters)
{
  return Slice<const C>::template tokenise<skipEmptyTokens>(onToken, delimiters);
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
template<size_t N>
inline MutableString<Size>::MutableString(const char(&str)[N])
  : Array<char, Size>(str, N)
{}

template<size_t Size>
inline MutableString<Size>::MutableString(Alloc_T, size_t count)
  : Array<char, Size>(Alloc, count)
{}
template<size_t Size>
inline MutableString<Size>::MutableString(Reserve_T, size_t count)
  : Array<char, Size>(Reserve, count)
{}
template<size_t Size>
template <typename... Args>
inline MutableString<Size>::MutableString(Concat_T, const Args&... args)
{
  if (sizeof...(Args) == 0)
    return;
  VarArg proxies[sizeof...(Args)+1] = { VarArg(args)... };
  appendInternal(Slice<VarArg>(proxies, sizeof...(Args)));
}
template<size_t Size>
template <typename... Args>
inline MutableString<Size>::MutableString(Format_T, String _format, const Args&... args)
{
  if (!_format)
    return;
  VarArg proxies[sizeof...(Args)+1] = { VarArg(args)... };
  formatInternal(_format, Slice<VarArg>(proxies, sizeof...(Args)));
}
template<size_t Size>
inline MutableString<Size>::MutableString(Sprintf_T, const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);

  size_t len = internal::epvscprintf(pFormat, args) + 1;

  this->reserve(len + 1);

  this->length = internal::epvsnprintf(this->ptr, len, pFormat, args);

  va_end(args);
}

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

  size_t len = internal::epvscprintf(pFormat, args) + 1;

  this->reserve(len + 1);

  this->length = internal::epvsnprintf(this->ptr, len, pFormat, args);

  va_end(args);

  return *this;
}

template<size_t Size>
inline MutableString<Size>& MutableString<Size>::toUpper()
{
  for (size_t i = 0; i < this->length; ++i)
    this->ptr[i] = epToUpper(this->ptr[i]);

  return *this;
}

template<size_t Size>
inline MutableString<Size>& MutableString<Size>::toLower()
{
  for (size_t i = 0; i < this->length; ++i)
    this->ptr[i] = epToLower(this->ptr[i]);

  return *this;
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

template <size_t Len>
inline SharedString::SharedString(const MutableString<Len> &str)
  : SharedArray<const char>((const Array<char, Len>&)str)
{}
template <size_t Len>
inline SharedString::SharedString(MutableString<Len> &&rval)
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
template<size_t N>
inline SharedString::SharedString(const char(&str)[N])
  : SharedArray<const char>(str, N)
{}

template <typename... Args>
inline SharedString::SharedString(Concat_T, const Args&... args)
{
  if (sizeof...(Args) == 0)
    return;
  VarArg proxies[sizeof...(Args)+1] = { VarArg(args)... };
  epConstruct(this) SharedString(concatInternal(Slice<VarArg>(proxies, sizeof...(Args))));
}
template <typename... Args>
inline SharedString::SharedString(Format_T, String _format, const Args&... args)
{
  if (!_format)
    return;
  VarArg proxies[sizeof...(Args)+1] = { VarArg(args)... };
  epConstruct(this) SharedString(formatInternal(_format, Slice<VarArg>(proxies, sizeof...(Args))));
}
inline SharedString::SharedString(Sprintf_T, const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);

  size_t len = internal::epvscprintf(pFormat, args) + 1;

  MutableString<0> s(Reserve, len);

  s.length = internal::epvsnprintf(s.ptr, len, pFormat, args);

  va_end(args);

  epConstruct(this) SharedString(std::move(s));
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
    static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs)
    {
      using ::epStringify;
      return epStringify(buffer, format, *(T*)pData, pArgs);
    }
    static const size_t intify = 0;
  };

  // make the numeric types promote explicitly
  template<> struct StringifyProxy<bool>     { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, *(bool*)pData, pArgs); }               inline static int64_t intify(const void *pData) { return *(bool*)pData ? 1 : 0; } };
  template<> struct StringifyProxy<uint8_t>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, (uint64_t)*(uint8_t*)pData, pArgs); }  inline static int64_t intify(const void *pData) { return (int64_t)*(uint8_t*)pData; } };
  template<> struct StringifyProxy<int8_t>   { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, (int64_t)*(int8_t*)pData, pArgs); }    inline static int64_t intify(const void *pData) { return (int64_t)*(int8_t*)pData; } };
  template<> struct StringifyProxy<uint16_t> { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, (uint64_t)*(uint16_t*)pData, pArgs); } inline static int64_t intify(const void *pData) { return (int64_t)*(uint16_t*)pData; } };
  template<> struct StringifyProxy<int16_t>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, (int64_t)*(int16_t*)pData, pArgs); }   inline static int64_t intify(const void *pData) { return (int64_t)*(int16_t*)pData; } };
  template<> struct StringifyProxy<uint32_t> { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, (uint64_t)*(uint32_t*)pData, pArgs); } inline static int64_t intify(const void *pData) { return (int64_t)*(uint32_t*)pData; } };
  template<> struct StringifyProxy<int32_t>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, (int64_t)*(int32_t*)pData, pArgs); }   inline static int64_t intify(const void *pData) { return (int64_t)*(int32_t*)pData; } };
  template<> struct StringifyProxy<uint64_t> { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, *(uint64_t*)pData, pArgs); }           inline static int64_t intify(const void *pData) { return (int64_t)*(uint64_t*)pData; } };
  template<> struct StringifyProxy<int64_t>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, *(int64_t*)pData, pArgs); }            inline static int64_t intify(const void *pData) { return *(int64_t*)pData; } };
  template<> struct StringifyProxy<float>    { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, (double)*(float*)pData, pArgs); }      inline static int64_t intify(const void *pData) { return (int64_t)*(float*)pData; } };
  template<> struct StringifyProxy<double>   { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, *(double*)pData, pArgs); }             inline static int64_t intify(const void *pData) { return (int64_t)*(double*)pData; } };

  template<> struct StringifyProxy<char*>           { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const char *pS = *(char**)pData;               return ::epStringify(buffer, format, String(pS, pS ? epStrlen(pS) : 0), pArgs); }  static const size_t intify = 0; };
  template<> struct StringifyProxy<char16_t*>       { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const char16_t *pS = *(char16_t**)pData;       return ::epStringify(buffer, format, WString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
  template<> struct StringifyProxy<char32_t*>       { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const char32_t *pS = *(char32_t**)pData;       return ::epStringify(buffer, format, DString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
  template<> struct StringifyProxy<const char*>     { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const char *pS = *(const char**)pData;         return ::epStringify(buffer, format, String(pS, pS ? epStrlen(pS) : 0), pArgs); }  static const size_t intify = 0; };
  template<> struct StringifyProxy<const char16_t*> { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const char16_t *pS = *(const char16_t**)pData; return ::epStringify(buffer, format, WString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
  template<> struct StringifyProxy<const char32_t*> { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const char32_t *pS = *(const char32_t**)pData; return ::epStringify(buffer, format, DString(pS, pS ? epStrlen(pS) : 0), pArgs); } static const size_t intify = 0; };
  template<size_t N>
  struct StringifyProxy<char[N]>                  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const char *pS = (const char*)pData;         return ::epStringify(buffer, format, String(pS, N-1), pArgs); }  static const size_t intify = 0; };
  template<size_t N>
  struct StringifyProxy<char16_t[N]>              { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const char16_t *pS = (const char16_t*)pData; return ::epStringify(buffer, format, WString(pS, N-1), pArgs); } static const size_t intify = 0; };
  template<size_t N>
  struct StringifyProxy<char32_t[N]>              { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const char32_t *pS = (const char32_t*)pData; return ::epStringify(buffer, format, DString(pS, N-1), pArgs); } static const size_t intify = 0; };
  template<size_t N>
  struct StringifyProxy<MutableString<N>>         { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, *(String*)pData, pArgs); } static const size_t intify = 0; };
  template<> struct StringifyProxy<SharedString>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return ::epStringify(buffer, format, *(String*)pData, pArgs); } static const size_t intify = 0; };

  size_t getLength(Slice<VarArg> args);
  Slice<char> concatenate(Slice<char> buffer, Slice<VarArg> args);
  Slice<char> format(String format, Slice<char> buffer, Slice<VarArg> args);
  size_t urlDecode(Slice<char> outSlice, String inStr);

} // namespace internal

template<typename T>
VarArg::VarArg(const T& arg)
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
  VarArg proxies[sizeof...(Args)+1] = { VarArg(args)... };
  appendInternal(Slice<VarArg>(proxies, sizeof...(Args)));
  return *this;
}
template<size_t Size>
template<typename... Args>
inline MutableString<Size>& MutableString<Size>::append(const Args&... args)
{
  VarArg proxies[sizeof...(Args)+1] = { VarArg(args)... };
  appendInternal(Slice<VarArg>(proxies, sizeof...(Args)));
  return *this;
}
template<size_t Size>
template<typename... Args>
inline MutableString<Size>& MutableString<Size>::format(String format, const Args&... args)
{
  VarArg proxies[sizeof...(Args)+1] = { VarArg(args)... };
  formatInternal(format, Slice<VarArg>(proxies, sizeof...(Args)));
  return *this;
}

template<size_t Size>
inline void MutableString<Size>::appendInternal(Slice<VarArg> args)
{
  size_t len = internal::getLength(args);
  this->reserve(this->length + len + 1);
  internal::concatenate(Slice<char>(this->ptr + this->length, len), args);
  this->length += len;
  this->ptr[this->length] = 0;
}
template<size_t Size>
inline void MutableString<Size>::formatInternal(String format, Slice<VarArg> args)
{
  size_t len = internal::format(format, nullptr, args).length;
  this->reserve(len + 1);
  internal::format(format, Slice<char>(this->ptr, len), args);
  this->length = len;
  this->ptr[len] = 0;
}

template<size_t Size>
inline MutableString<Size>& MutableString<Size>::urlDecode(String s)
{
  this->reserve(s.length);
  this->length = internal::urlDecode(Slice<char>(this->ptr, s.length), s);

  return *this;
}

// SharedString

template<typename... Args>
inline SharedString SharedString::concat(const Args&... args)
{
  return SharedString(Concat, args...);
}
template<typename... Args>
inline SharedString SharedString::format(String _format, const Args&... args)
{
  return SharedString(Format, _format, args...);
}

} // namespace ep

template<typename T>
epforceinline ptrdiff_t epStringifyTemplate(ep::Slice<char> buffer, ep::String format, const T &val, const ep::VarArg *pArgs)
{
  return ep::internal::StringifyProxy<T>::stringify(buffer, format, (void*)&val, pArgs);
}
