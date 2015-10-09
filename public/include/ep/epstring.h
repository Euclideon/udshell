#pragma once
#if !defined(_EP_STRING)
#define _EP_STRING

#include "ep/epslice.h"

namespace ep {
namespace internal {

  extern const char charDetails[256];

} // namespace internal
} // namespace ep

#define isNewline(c) (c < 256 && (ep::internal::charDetails[c] & 8))
#define isWhitespace(c) (c < 256 && (ep::internal::charDetails[c] & 0xC))
#define isAlpha(c) (c < 256 && (ep::internal::charDetails[c] & 1))
#define isNumeric(c) (c < 256 && (ep::internal::charDetails[c] & 2))
#define isAlphaNumeric(c) (c < 256 && (ep::internal::charDetails[c] & 3))
#define isHex(c) (isAlphaNumeric(c) && (c|0x20) <= 'f')

epforceinline char toLower(char c) { return isAlpha(c) ? c|0x20 : c; }
epforceinline char toUpper(char c) { return isAlpha(c) ? c&~0x20 : c; }

#if !defined(__cplusplus)

// C compilers just define epString as a simple struct
struct epString
{
  size_t length;
  const char *ptr;
};

#else

template<typename C>
class epCString;
struct epVarArg;


// epBaseString implements an immutable string; a layer above epSlice<>, defined for 'const C' (ie, char, char16_t, char32_t) and adds string-specific methods
// epBaseString<C> and epSlice<C> are fully compatible
// does not retain ownership; useful for local temporaries, passing as arguments, etc... analogous to using 'const char*'
// epBaseString adds typical string functions, including conversion to zero-terminated strings for passing to foreign APIs, akin to c_str()
template<typename C>
struct epBaseString : public epSlice<const C>
{
  // constructors
  epBaseString();
  epBaseString(const C *ptr, size_t length);
  template<typename C2>
  epBaseString(epSlice<C2> rh);
  epBaseString(const C *pString);
  template<size_t N>
  epBaseString(const C str[N]);

  // assignment
  epBaseString<C>& operator =(epSlice<const C> rh);
  epBaseString<C>& operator =(const C *pString);

  // contents
  epBaseString<C> slice(ptrdiff_t first, ptrdiff_t last) const;

  // unicode support
  size_t numChars() const;
  char32_t frontChar() const;
  char32_t backChar() const;
  char32_t popFrontChar();
  char32_t popBackChar();

  // comparison
  bool eq(epBaseString<C> rh) const;
  bool eqIC(epBaseString<C> rh) const;
  bool beginsWith(epBaseString<C> rh) const;
  bool beginsWithIC(epBaseString<C> rh) const;
  bool endsWith(epBaseString<C> rh) const;
  bool endsWithIC(epBaseString<C> rh) const;

  ptrdiff_t cmp(epBaseString<C> rh) const;
  ptrdiff_t cmpIC(epBaseString<C> rh) const;

  // c-string compatibility
  C* toStringz(C *pBuffer, size_t bufferLen) const;
  epCString<C> toStringz() const;

  // useful functions
  size_t findFirstIC(epBaseString<C> s) const;
  size_t findLastIC(epBaseString<C> s) const;

  epBaseString<C> getLeftAtFirstIC(epBaseString<C> s, bool bInclusive = false) const;
  epBaseString<C> getLeftAtLastIC(epBaseString<C> s, bool bInclusive = false) const;
  epBaseString<C> getRightAtFirstIC(epBaseString<C> s, bool bInclusive = true) const;
  epBaseString<C> getRightAtLastIC(epBaseString<C> s, bool bInclusive = true) const;

  template<bool Front = true, bool Back = true>
  epBaseString<C> trim() const;

  template<bool skipEmptyTokens = true>
  epBaseString<C> popToken(epBaseString<C> delimiters = " \t\r\n");
  template<bool skipEmptyTokens = true>
  epSlice<epBaseString<C>> tokenise(epSlice<epBaseString<C>> tokens, epBaseString<C> delimiters = " \t\r\n");

  int64_t parseInt(bool bDetectBase = true, int base = 10) const;
  double parseFloat() const;

  uint32_t hash(uint32_t hash = 0) const;
};

typedef epBaseString<char> epString;
typedef epBaseString<char16_t> epWString;
typedef epBaseString<char32_t> epDString;


// a static-length and/or stack-allocated string, useful for holding constructured temporaries (ie, target for sprintf/format)
// alternatively useful for holding a string as a member of an aggregate without breaking out to separate allocations
// useful in cases where 'char buffer[len]' is typically found
template<size_t Size>
struct epMutableString : public epArray<char, Size>
{
  // constructors
  epMutableString();
  epMutableString(epMutableString<Size> &&rval);
  epMutableString(epArray<char, Size> &&rval);
  template <typename U>
  epMutableString(U *ptr, size_t length);
  template <typename U>
  epMutableString(epSlice<U> slice);
  epMutableString(const char *pString);

  // assignment
  epMutableString& operator =(epMutableString<Size> &&rval);
  epMutableString& operator =(epArray<char, Size> &&rval);
  template <typename U>
  epMutableString& operator =(epSlice<U> rh);
  epMutableString& operator =(const char *pString);

  // contents
  epString slice(ptrdiff_t first, ptrdiff_t last) const                                 { return ((epString*)this)->slice(first, last); }

  // comparison
  bool eq(epString rh) const                                                            { return ((epString*)this)->eq(rh); }
  bool eqIC(epString rh) const                                                          { return ((epString*)this)->eqIC(rh); }
  bool beginsWith(epString rh) const                                                    { return ((epString*)this)->beginsWith(rh); }
  bool beginsWithIC(epString rh) const                                                  { return ((epString*)this)->beginsWithIC(rh); }
  bool endsWith(epString rh) const                                                      { return ((epString*)this)->endsWith(rh); }
  bool endsWithIC(epString rh) const                                                    { return ((epString*)this)->endsWithIC(rh); }

  ptrdiff_t cmp(epString rh) const                                                      { return ((epString*)this)->cmp(rh); }
  ptrdiff_t cmpIC(epString rh) const                                                    { return ((epString*)this)->cmpIC(rh); }

  // manipulation
  template<typename... Args> epMutableString& concat(const Args&... args);
  template<typename... Args> epMutableString& append(const Args&... args);

  template<typename... Args> epMutableString& format(epString format, const Args&... args);
  epMutableString& sprintf(const char *pFormat, ...)  epprintf_func(2, 3);

  epMutableString& toUpper();
  epMutableString& toLower();

  // c-string compatibility
  char* toStringz(char *pBuffer, size_t bufferLen) const                                { return ((epString*)this)->toStringz(pBuffer, bufferLen); }
  epCString<char> toStringz() const;

  // useful functions
  size_t findFirstIC(epString s) const                                                  { return ((epString*)this)->findFirstIC(s); }
  size_t findLastIC(epString s) const                                                   { return ((epString*)this)->findLastIC(s); }

  epString getLeftAtFirstIC(epString s, bool bInclusive = false) const                  { return ((epString*)this)->getLeftAtFirstIC(s, bInclusive); }
  epString getLeftAtLastIC(epString s, bool bInclusive = false) const                   { return ((epString*)this)->getLeftAtLastIC(s, bInclusive); }
  epString getRightAtFirstIC(epString s, bool bInclusive = true) const                  { return ((epString*)this)->getRightAtFirstIC(s, bInclusive); }
  epString getRightAtLastIC(epString s, bool bInclusive = true) const                   { return ((epString*)this)->getRightAtLastIC(s, bInclusive); }

  template<bool Front = true, bool Back = true>
  epString trim() const                                                                 { return ((epString*)this)->trim<Front, Back>(); }

  template<bool skipEmptyTokens = true>
  epString popToken(epString delimiters = " \t\r\n")                                    { return ((epString*)this)->popToken<skipEmptyTokens>(delimiters); }
  template<bool skipEmptyTokens = true>
  epSlice<epString> tokenise(epSlice<epString> tokens, epString delimiters = " \t\r\n") { return ((epString*)this)->tokenise<skipEmptyTokens>(tokens, delimiters); }

  int64_t parseInt(bool bDetectBase = true, int base = 10) const                        { return ((epString*)this)->parseInt(bDetectBase, base); }
  double parseFloat() const                                                             { return ((epString*)this)->parseFloat(); }

  uint32_t hash(uint32_t hash = 0) const                                                { return ((epString*)this)->hash(hash); }

private:
  void appendInternal(epSlice<epVarArg> args);
  void formatInternal(epString format, epSlice<epVarArg> args);
};

// we'll typedef these such that the desired size compensates for the other internal members
typedef epMutableString<64 - sizeof(epSlice<char>)> epMutableString64;
typedef epMutableString<128 - sizeof(epSlice<char>)> epMutableString128;
typedef epMutableString<256 - sizeof(epSlice<char>)> epMutableString256;


// reference-counted allocated string, used to retain ownership of arbitrary length strings
// reference counting allows allocations to be shared between multiple owners
// useful in situations where std::string would usually be found, but without the endless allocation and copying or linkage problems
struct epSharedString : public epSharedSlice<const char>
{
  // constructors
  epSharedString();
  epSharedString(epSharedString &&rval);
  epSharedString(const epSharedString &val);
  epSharedString(epSharedSlice<const char> &&rval);
  epSharedString(const epSharedSlice<const char> &rcstr);
  template <typename U>
  epSharedString(U *ptr, size_t length);
  template <typename U>
  epSharedString(epSlice<U> slice);
  epSharedString(const char *pString);

  // construction
  template<typename... Args> static epSharedString concat(const Args&... args);
  template<typename... Args> static epSharedString format(epString format, const Args&... args);
  static epSharedString sprintf(const char *pFormat, ...) epprintf_func(1, 2);

  // assignment
  epSharedString& operator =(const epSharedSlice<const char> &rh);
  epSharedString& operator =(const epSharedString &rval);
  epSharedString& operator =(epSharedString &&rval);
  epSharedString& operator =(epSharedSlice<const char> &&rval);
  template <typename U>
  epSharedString& operator =(epSlice<U> rh);
  epSharedString& operator =(const char *pString);

  // contents
  epSharedString slice(ptrdiff_t first, ptrdiff_t last) const;

  // comparison
  bool eq(epString rh) const                                                            { return ((epString*)this)->eq(rh); }
  bool eqIC(epString rh) const                                                          { return ((epString*)this)->eqIC(rh); }
  bool beginsWith(epString rh) const                                                    { return ((epString*)this)->beginsWith(rh); }
  bool beginsWithIC(epString rh) const                                                  { return ((epString*)this)->beginsWithIC(rh); }
  bool endsWith(epString rh) const                                                      { return ((epString*)this)->endsWith(rh); }
  bool endsWithIC(epString rh) const                                                    { return ((epString*)this)->endsWithIC(rh); }

  ptrdiff_t cmp(epString rh) const                                                      { return ((epString*)this)->cmp(rh); }
  ptrdiff_t cmpIC(epString rh) const                                                    { return ((epString*)this)->cmpIC(rh); }

  // c-string compatibility
  char* toStringz(char *pBuffer, size_t bufferLen) const                                { return ((epString*)this)->toStringz(pBuffer, bufferLen); }
  epCString<char> toStringz() const;

  // transformation
  template<typename... Args> epSharedString append(const Args&... args) const           { return concat(*this, args...); }

  epSharedString asUpper() const;
  epSharedString asLower() const;

  // useful functions
  size_t findFirstIC(epString s) const                                                  { return ((epString*)this)->findFirstIC(s); }
  size_t findLastIC(epString s) const                                                   { return ((epString*)this)->findLastIC(s); }

  epSharedString getLeftAtFirstIC(epString s, bool bInclusive = false) const;
  epSharedString getLeftAtLastIC(epString s, bool bInclusive = false) const;
  epSharedString getRightAtFirstIC(epString s, bool bInclusive = true) const;
  epSharedString getRightAtLastIC(epString s, bool bInclusive = true) const;

  template<bool Front = true, bool Back = true>
  epSharedString trim() const;

  template<bool skipEmptyTokens = true>
  epSharedString popToken(epString delimiters = " \t\r\n");
  template<bool skipEmptyTokens = true>
  epSlice<epSharedString> tokenise(epSlice<epSharedString> tokens, epString delimiters = " \t\r\n") const;

  int64_t parseInt(bool bDetectBase = true, int base = 10) const                        { return ((epString*)this)->parseInt(bDetectBase, base); }
  double parseFloat() const                                                             { return ((epString*)this)->parseFloat(); }

  uint32_t hash(uint32_t hash = 0) const                                                { return ((epString*)this)->hash(hash); }

private:
  epSharedString(const char *ptr, size_t length, epRC *rc);

  static epSharedString concatInternal(epSlice<epVarArg> args);
  static epSharedString formatInternal(epString format, epSlice<epVarArg> args);
};


// vararg for stringification
struct epVarArg
{
  epVarArg() {}
  template<typename T>
  epVarArg(const T& arg);

  bool HasIntify() const { return pIntProxy != nullptr; }

  ptrdiff_t GetString(epSlice<char> buffer, epString format = nullptr, const epVarArg *pArgs = nullptr) const { return pStringProxy(buffer, format, pArg, pArgs); }
  ptrdiff_t GetStringLength(epString format = nullptr, const epVarArg *pArgs = nullptr) const { return pStringProxy(nullptr, format, pArg, pArgs); }
  int64_t GetInt() const { return pIntProxy ? pIntProxy(pArg) : 0; }

private:
  typedef ptrdiff_t(StringifyFunc)(epSlice<char>, epString, const void*, const epVarArg*);
  typedef int64_t(IntConvFunc)(const void*);

  const void *pArg;
  StringifyFunc *pStringProxy;
  IntConvFunc *pIntProxy;
};


// unit tests
udResult epString_Test();

#endif

#endif // _EP_STRING
