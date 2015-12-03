#pragma once
#if !defined(_EP_STRING_HPP)
#define _EP_STRING_HPP

#include "ep/c/string.h"

#include "ep/cpp/slice.h"

namespace ep {

epforceinline char toLower(char c) { return epIsAlpha(c) ? c|0x20 : c; }
epforceinline char toUpper(char c) { return epIsAlpha(c) ? c&~0x20 : c; }


enum Format_T { Format };
enum Sprintf_T { Sprintf };


template<typename C>
class CString;
struct epVarArg;


// BaseString implements an immutable string; a layer above Slice<>, defined for 'const C' (ie, char, char16_t, char32_t) and adds string-specific methods
// BaseString<C> and Slice<C> are fully compatible
// does not retain ownership; useful for local temporaries, passing as arguments, etc... analogous to using 'const char*'
// BaseString adds typical string functions, including conversion to zero-terminated strings for passing to foreign APIs, akin to c_str()
template<typename C>
struct BaseString : public Slice<const C>
{
  // constructors
  BaseString();
  BaseString(const C *ptr, size_t length);
  template<typename C2>
  BaseString(Slice<C2> rh);
  BaseString(const C *pString);
  template<size_t N>
  BaseString(const C str[N]);

  // epString compatibility
  BaseString(epString s);
  operator epString() const;

  // assignment
  BaseString<C>& operator =(Slice<const C> rh);
  BaseString<C>& operator =(const C *pString);

  // contents
  BaseString<C> slice(ptrdiff_t first, ptrdiff_t last) const;

  // unicode support
  size_t numChars() const;
  char32_t frontChar() const;
  char32_t backChar() const;
  char32_t popFrontChar();
  char32_t popBackChar();

  // comparison
  bool eq(BaseString<C> rh) const;
  bool eqIC(BaseString<C> rh) const;
  bool beginsWith(BaseString<C> rh) const;
  bool beginsWithIC(BaseString<C> rh) const;
  bool endsWith(BaseString<C> rh) const;
  bool endsWithIC(BaseString<C> rh) const;

  ptrdiff_t cmp(BaseString<C> rh) const;
  ptrdiff_t cmpIC(BaseString<C> rh) const;

  // c-string compatibility
  C* toStringz(C *pBuffer, size_t bufferLen) const;
  CString<C> toStringz() const;

  // useful functions

  // make base functions local
  size_t findFirst(C c) const                                                         { return ((Slice<const C>*)this)->findFirst(c); }
  size_t findLast(C c) const                                                          { return ((Slice<const C>*)this)->findLast(c); }
  size_t findFirst(BaseString<C> s) const                                             { return ((Slice<const C>*)this)->findFirst(s); }
  size_t findLast(BaseString<C> s) const                                              { return ((Slice<const C>*)this)->findLast(s); }
  BaseString<C> getLeftAtFirst(C c, bool bInclusive = false) const                    { return ((Slice<const C>*)this)->getLeftAtFirst(c, bInclusive); }
  BaseString<C> getLeftAtLast(C c, bool bInclusive = false) const                     { return ((Slice<const C>*)this)->getLeftAtLast(c, bInclusive); }
  BaseString<C> getRightAtFirst(C c, bool bInclusive = true) const                    { return ((Slice<const C>*)this)->getRightAtFirst(c, bInclusive); }
  BaseString<C> getRightAtLast(C c, bool bInclusive = true) const                     { return ((Slice<const C>*)this)->getRightAtLast(c, bInclusive); }
  BaseString<C> getLeftAtFirst(BaseString<C> s, bool bInclusive = false) const        { return ((Slice<const C>*)this)->getLeftAtFirst(s, bInclusive); }
  BaseString<C> getLeftAtLast(BaseString<C> s, bool bInclusive = false) const         { return ((Slice<const C>*)this)->getLeftAtLast(s, bInclusive); }
  BaseString<C> getRightAtFirst(BaseString<C> s, bool bInclusive = true) const        { return ((Slice<const C>*)this)->getRightAtFirst(s, bInclusive); }
  BaseString<C> getRightAtLast(BaseString<C> s, bool bInclusive = true) const         { return ((Slice<const C>*)this)->getRightAtLast(s, bInclusive); }

  // insensitice case versions
  size_t findFirstIC(BaseString<C> s) const;
  size_t findLastIC(BaseString<C> s) const;
  BaseString<C> getLeftAtFirstIC(BaseString<C> s, bool bInclusive = false) const;
  BaseString<C> getLeftAtLastIC(BaseString<C> s, bool bInclusive = false) const;
  BaseString<C> getRightAtFirstIC(BaseString<C> s, bool bInclusive = true) const;
  BaseString<C> getRightAtLastIC(BaseString<C> s, bool bInclusive = true) const;

  template<bool Front = true, bool Back = true>
  BaseString<C> trim() const;

  template<bool skipEmptyTokens = true>
  BaseString<C> popToken(BaseString<C> delimiters = " \t\r\n");
  template<bool skipEmptyTokens = true>
  Slice<BaseString<C>> tokenise(Slice<BaseString<C>> tokens, BaseString<C> delimiters = " \t\r\n");

  int64_t parseInt(bool bDetectBase = true, int base = 10) const;
  double parseFloat() const;

  uint32_t hash(uint32_t hash = 0) const;
};

typedef BaseString<char> String;
typedef BaseString<char16_t> WString;
typedef BaseString<char32_t> DString;


// a static-length and/or stack-allocated string, useful for holding constructured temporaries (ie, target for sprintf/format)
// alternatively useful for holding a string as a member of an aggregate without breaking out to separate allocations
// useful in cases where 'char buffer[len]' is typically found
template<size_t Size>
struct MutableString : public Array<char, Size>
{
  // constructors
  MutableString();
  MutableString(const MutableString<Size> &rh);
  MutableString(MutableString<Size> &&rval);
  template <size_t Len> MutableString(Array<char, Len> &&rval);
  template <typename U> MutableString(U *ptr, size_t length);
  template <typename U> MutableString(Slice<U> slice);
  MutableString(const char *pString);

  MutableString(Alloc_T, size_t count);
  MutableString(Reserve_T, size_t count);
  template <typename... Args> MutableString(Concat_T, const Args&... args);
  template <typename... Args> MutableString(Format_T, String format, const Args&... args);
  MutableString(Sprintf_T, const char *pFormat, ...) epprintf_func(3, 4);

  // assignment
  MutableString& operator =(const MutableString<Size> &rh);
  MutableString& operator =(MutableString<Size> &&rval);
  template <size_t Len> MutableString& operator =(Array<char, Len> &&rval);
  template <typename U> MutableString& operator =(Slice<U> rh);
  MutableString& operator =(const char *pString);

  // contents
  String slice(ptrdiff_t first, ptrdiff_t last) const                                 { return ((String*)this)->slice(first, last); }

  // comparison
  bool eq(String rh) const                                                            { return ((String*)this)->eq(rh); }
  bool eqIC(String rh) const                                                          { return ((String*)this)->eqIC(rh); }
  bool beginsWith(String rh) const                                                    { return ((String*)this)->beginsWith(rh); }
  bool beginsWithIC(String rh) const                                                  { return ((String*)this)->beginsWithIC(rh); }
  bool endsWith(String rh) const                                                      { return ((String*)this)->endsWith(rh); }
  bool endsWithIC(String rh) const                                                    { return ((String*)this)->endsWithIC(rh); }

  ptrdiff_t cmp(String rh) const                                                      { return ((String*)this)->cmp(rh); }
  ptrdiff_t cmpIC(String rh) const                                                    { return ((String*)this)->cmpIC(rh); }

  // manipulation
  template<typename... Args> MutableString& concat(const Args&... args);
  template<typename... Args> MutableString& append(const Args&... args);

  template<typename... Args> MutableString& format(String format, const Args&... args);
  MutableString& sprintf(const char *pFormat, ...)  epprintf_func(2, 3);

  MutableString& toUpper();
  MutableString& toLower();

  // c-string compatibility
  char* toStringz(char *pBuffer, size_t bufferLen) const                              { return ((String*)this)->toStringz(pBuffer, bufferLen); }
  CString<char> toStringz() const;

  // useful functions
  size_t findFirstIC(String s) const                                                  { return ((String*)this)->findFirstIC(s); }
  size_t findLastIC(String s) const                                                   { return ((String*)this)->findLastIC(s); }

  String getLeftAtFirstIC(String s, bool bInclusive = false) const                    { return ((String*)this)->getLeftAtFirstIC(s, bInclusive); }
  String getLeftAtLastIC(String s, bool bInclusive = false) const                     { return ((String*)this)->getLeftAtLastIC(s, bInclusive); }
  String getRightAtFirstIC(String s, bool bInclusive = true) const                    { return ((String*)this)->getRightAtFirstIC(s, bInclusive); }
  String getRightAtLastIC(String s, bool bInclusive = true) const                     { return ((String*)this)->getRightAtLastIC(s, bInclusive); }

  template<bool Front = true, bool Back = true>
  String trim() const                                                                 { return ((String*)this)->trim<Front, Back>(); }

  template<bool skipEmptyTokens = true>
  Slice<String> tokenise(Slice<String> tokens, String delimiters = " \t\r\n")         { return ((String*)this)->tokenise<skipEmptyTokens>(tokens, delimiters); }

  int64_t parseInt(bool bDetectBase = true, int base = 10) const                      { return ((String*)this)->parseInt(bDetectBase, base); }
  double parseFloat() const                                                           { return ((String*)this)->parseFloat(); }

  uint32_t hash(uint32_t hash = 0) const                                              { return ((String*)this)->hash(hash); }

private:
  void appendInternal(Slice<epVarArg> args);
  void formatInternal(String format, Slice<epVarArg> args);
};

// we'll typedef these such that the desired size compensates for the other internal members
typedef MutableString<64 - sizeof(Slice<char>)> MutableString64;
typedef MutableString<128 - sizeof(Slice<char>)> MutableString128;
typedef MutableString<256 - sizeof(Slice<char>)> MutableString256;


// reference-counted allocated string, used to retain ownership of arbitrary length strings
// reference counting allows allocations to be shared between multiple owners
// useful in situations where std::string would usually be found, but without the endless allocation and copying or linkage problems
struct SharedString : public SharedArray<const char>
{
  // constructors
  SharedString();
  SharedString(const SharedString &val);
  SharedString(SharedString &&rval);
  SharedString(const SharedArray<const char> &rcstr);
  SharedString(SharedArray<const char> &&rval);
  template <size_t Len> SharedString(const MutableString<Len> &arr);
  template <size_t Len> SharedString(MutableString<Len> &&rval);
  template <typename U> SharedString(U *ptr, size_t length);
  template <typename U> SharedString(Slice<U> slice);
  SharedString(const char *pString);

  template <typename... Args> SharedString(Concat_T, const Args&... args);
  template <typename... Args> SharedString(Format_T, String format, const Args&... args);
  SharedString(Sprintf_T, const char *pFormat, ...) epprintf_func(3, 4);

  // epString compatibility
  SharedString(const epSharedString &s);
  SharedString(epSharedString &&s);
  operator epSharedString() const;

  // construction
  template<typename... Args> static SharedString concat(const Args&... args);
  template<typename... Args> static SharedString format(String format, const Args&... args);
  static SharedString sprintf(const char *pFormat, ...) epprintf_func(1, 2);

  // assignment
  SharedString& operator =(const SharedString &rh);
  SharedString& operator =(SharedString &&rval);
  SharedString& operator =(const SharedArray<const char> &rh);
  SharedString& operator =(SharedArray<const char> &&rval);
  template <typename U> SharedString& operator =(Slice<U> rh);
  SharedString& operator =(const char *pString);

  // contents
  String slice(ptrdiff_t first, ptrdiff_t last) const                                 { return ((String*)this)->slice(first, last); }

  // comparison
  bool eq(String rh) const                                                            { return ((String*)this)->eq(rh); }
  bool eqIC(String rh) const                                                          { return ((String*)this)->eqIC(rh); }
  bool beginsWith(String rh) const                                                    { return ((String*)this)->beginsWith(rh); }
  bool beginsWithIC(String rh) const                                                  { return ((String*)this)->beginsWithIC(rh); }
  bool endsWith(String rh) const                                                      { return ((String*)this)->endsWith(rh); }
  bool endsWithIC(String rh) const                                                    { return ((String*)this)->endsWithIC(rh); }

  ptrdiff_t cmp(String rh) const                                                      { return ((String*)this)->cmp(rh); }
  ptrdiff_t cmpIC(String rh) const                                                    { return ((String*)this)->cmpIC(rh); }

  // c-string compatibility
  char* toStringz(char *pBuffer, size_t bufferLen) const                              { return ((String*)this)->toStringz(pBuffer, bufferLen); }
  CString<char> toStringz() const;

  // transformation
  template<typename... Args> SharedString append(const Args&... args) const           { return concat(*this, args...); }

  SharedString asUpper() const;
  SharedString asLower() const;

  // useful functions
  size_t findFirstIC(String s) const                                                  { return ((String*)this)->findFirstIC(s); }
  size_t findLastIC(String s) const                                                   { return ((String*)this)->findLastIC(s); }

  String getLeftAtFirstIC(String s, bool bInclusive = false) const                    { return ((String*)this)->getLeftAtFirstIC(s, bInclusive); }
  String getLeftAtLastIC(String s, bool bInclusive = false) const                     { return ((String*)this)->getLeftAtLastIC(s, bInclusive); }
  String getRightAtFirstIC(String s, bool bInclusive = true) const                    { return ((String*)this)->getRightAtFirstIC(s, bInclusive); }
  String getRightAtLastIC(String s, bool bInclusive = true) const                     { return ((String*)this)->getRightAtLastIC(s, bInclusive); }

  template<bool Front = true, bool Back = true>
  String trim() const                                                                 { return ((String*)this)->trim<Front, Back>(); }

  template<bool skipEmptyTokens = true>
  Slice<String> tokenise(Slice<String> tokens, String delimiters = " \t\r\n") const   { return ((String*)this)->tokenise<skipEmptyTokens>(tokens, delimiters); }

  int64_t parseInt(bool bDetectBase = true, int base = 10) const                      { return ((String*)this)->parseInt(bDetectBase, base); }
  double parseFloat() const                                                           { return ((String*)this)->parseFloat(); }

  uint32_t hash(uint32_t hash = 0) const                                              { return ((String*)this)->hash(hash); }

private:
  static SharedString concatInternal(Slice<epVarArg> args);
  static SharedString formatInternal(String format, Slice<epVarArg> args);
};


// vararg for stringification
struct epVarArg
{
  epVarArg() {}
  template<typename T>
  epVarArg(const T& arg);

  bool HasIntify() const { return pIntProxy != nullptr; }

  ptrdiff_t GetString(Slice<char> buffer, String format = nullptr, const epVarArg *pArgs = nullptr) const { return pStringProxy(buffer, format, pArg, pArgs); }
  ptrdiff_t GetStringLength(String format = nullptr, const epVarArg *pArgs = nullptr) const { return pStringProxy(nullptr, format, pArg, pArgs); }
  int64_t GetInt() const { return pIntProxy ? pIntProxy(pArg) : 0; }

private:
  typedef ptrdiff_t(StringifyFunc)(Slice<char>, String, const void*, const epVarArg*);
  typedef int64_t(IntConvFunc)(const void*);

  const void *pArg;
  StringifyFunc *pStringProxy;
  IntConvFunc *pIntProxy;
};

} // namespace ep

// unit tests
epResult epString_Test();

#endif // _EP_STRING_HPP
