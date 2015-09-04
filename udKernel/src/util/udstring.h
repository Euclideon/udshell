#if !defined(_UD_STRING)
#define _UD_STRING

#include "udslice.h"


extern const char s_charDetails[256];

#define isNewline(c) (s_charDetails[(uint8_t)c] & 8)
#define isWhitespace(c) (s_charDetails[(uint8_t)c] & 0xC)
#define isAlpha(c) (s_charDetails[(uint8_t)c] & 1)
#define isNumeric(c) (s_charDetails[(uint8_t)c] & 2)
#define isAlphaNumeric(c) (s_charDetails[(uint8_t)c] & 3)
#define isHex(c) (isAlphaNumeric(c) && (uint8_t(c)|0x20) <= 'F')

UDFORCE_INLINE char toLower(char c) { return isAlpha(c) ? c|0x20 : c; }
UDFORCE_INLINE char toUpper(char c) { return isAlpha(c) ? c&~0x20 : c; }

#if !defined(__cplusplus)

// C compilers just define udString as a simple struct
struct udString
{
  size_t length;
  const char *ptr;
};

#else

class udCString;
namespace ud_internal { struct VarArg; }


// udString is a layer above udSlice<>, defined for 'const char' (utf-8) and adds string-specific methods
// udString and udSlice<> are fully compatible

// udString implements an immutable string
// does not retain ownership; useful for local temporaries, passing as arguments, etc... analogous to using 'const char*'
// udString adds typical string functions, including conversion to zero-terminated strings for passing to foreign APIs, akin to c_str()
struct udString : public udSlice<const char>
{
  // constructors
  udString();
  udString(const char *ptr, size_t length);
  template<typename C>
  udString(udSlice<C> rh);
  udString(const char *pString);
  template<size_t N>
  udString(const char str[N]);

  // assignment
  udString& operator =(udSlice<const char> rh);
  udString& operator =(const char *pString);

  // contents
  udString slice(ptrdiff_t first, ptrdiff_t last) const;

  // comparison
  bool eq(udString rh) const;
  bool eqIC(udString rh) const;
  bool beginsWith(udString rh) const;
  bool beginsWithIC(udString rh) const;
  bool endsWith(udString rh) const;
  bool endsWithIC(udString rh) const;

  ptrdiff_t cmp(udString rh) const;
  ptrdiff_t cmpIC(udString rh) const;

  // c-string compatibility
  char* toStringz(char *pBuffer, size_t bufferLen) const;
  udCString toStringz() const;

  // useful functions
  size_t findFirstIC(udString s) const;
  size_t findLastIC(udString s) const;

  udString getLeftAtFirstIC(udString s, bool bInclusive = false) const;
  udString getLeftAtLastIC(udString s, bool bInclusive = false) const;
  udString getRightAtFirstIC(udString s, bool bInclusive = true) const;
  udString getRightAtLastIC(udString s, bool bInclusive = true) const;

  udString trim(bool front = true, bool back = true) const;

  template<bool skipEmptyTokens = true>
  udString popToken(udString delimiters = " \t\r\n");
  template<bool skipEmptyTokens = true>
  udSlice<udString> tokenise(udSlice<udString> tokens, udString delimiters = " \t\r\n");

  int64_t parseInt(bool bDetectBase = true, int base = 10) const;
  double parseFloat() const;

  uint32_t hash(uint32_t hash = 0) const;
};


// a static-length and/or stack-allocated string, useful for holding constructured temporaries (ie, target for sprintf/format)
// alternatively useful for holding a string as a member of an aggregate without breaking out to separate allocations
// useful in cases where 'char buffer[len]' is typically found
template<size_t Size>
struct udMutableString : public udFixedSlice<char, Size>
{
  // constructors
  udMutableString();
  udMutableString(udMutableString<Size> &&rval);
  udMutableString(udFixedSlice<char, Size> &&rval);
  template <typename U>
  udMutableString(U *ptr, size_t length);
  template <typename U>
  udMutableString(udSlice<U> slice);
  udMutableString(const char *pString);

  // assignment
  udMutableString& operator =(udMutableString<Size> &&rval);
  udMutableString& operator =(udFixedSlice<char, Size> &&rval);
  template <typename U>
  udMutableString& operator =(udSlice<U> rh);
  udMutableString& operator =(const char *pString);

  // contents
  udString slice(ptrdiff_t first, ptrdiff_t last) const                                 { return ((udString*)this)->slice(first, last); }

  // comparison
  bool eq(udString rh) const                                                            { return ((udString*)this)->eq(rh); }
  bool eqIC(udString rh) const                                                          { return ((udString*)this)->eqIC(rh); }
  bool beginsWith(udString rh) const                                                    { return ((udString*)this)->beginsWith(rh); }
  bool beginsWithIC(udString rh) const                                                  { return ((udString*)this)->beginsWithIC(rh); }
  bool endsWith(udString rh) const                                                      { return ((udString*)this)->endsWith(rh); }
  bool endsWithIC(udString rh) const                                                    { return ((udString*)this)->endsWithIC(rh); }

  ptrdiff_t cmp(udString rh) const                                                      { return ((udString*)this)->cmp(rh); }
  ptrdiff_t cmpIC(udString rh) const                                                    { return ((udString*)this)->cmpIC(rh); }

  // manipulation
  template<typename... Args> udMutableString& concat(const Args&... args);
  template<typename... Args> udMutableString& append(const Args&... args);

  template<typename... Args> udMutableString& format(udString format, const Args&... args);
  udMutableString& sprintf(const char *pFormat, ...);

  udMutableString& toUpper();
  udMutableString& toLower();

  // c-string compatibility
  char* toStringz(char *pBuffer, size_t bufferLen) const                                { return ((udString*)this)->toStringz(pBuffer, bufferLen); }
  udCString toStringz() const;

  // useful functions
  size_t findFirstIC(udString s) const                                                  { return ((udString*)this)->findFirstIC(s); }
  size_t findLastIC(udString s) const                                                   { return ((udString*)this)->findLastIC(s); }

  udString getLeftAtFirstIC(udString s, bool bInclusive = false) const                  { return ((udString*)this)->getLeftAtFirstIC(s, bInclusive); }
  udString getLeftAtLastIC(udString s, bool bInclusive = false) const                   { return ((udString*)this)->getLeftAtLastIC(s, bInclusive); }
  udString getRightAtFirstIC(udString s, bool bInclusive = true) const                  { return ((udString*)this)->getRightAtFirstIC(s, bInclusive); }
  udString getRightAtLastIC(udString s, bool bInclusive = true) const                   { return ((udString*)this)->getRightAtLastIC(s, bInclusive); }

  udString trim(bool front = true, bool back = true) const                              { return ((udString*)this)->trim(front, back); }

  template<bool skipEmptyTokens = true>
  udString popToken(udString delimiters = " \t\r\n")                                    { return ((udString*)this)->popToken<skipEmptyTokens>(delimiters); }
  template<bool skipEmptyTokens = true>
  udSlice<udString> tokenise(udSlice<udString> tokens, udString delimiters = " \t\r\n") { return ((udString*)this)->tokenise<skipEmptyTokens>(tokens, delimiters); }

  int64_t parseInt(bool bDetectBase = true, int base = 10) const                        { return ((udString*)this)->parseInt(bDetectBase, base); }
  double parseFloat() const                                                             { return ((udString*)this)->parseFloat(); }

  uint32_t hash(uint32_t hash = 0) const                                                { return ((udString*)this)->hash(hash); }

private:
  void appendInternal(udSlice<ud_internal::VarArg> args);
  void formatInternal(udString format, udSlice<ud_internal::VarArg> args);
};

// we'll typedef these such that the desired size compensates for the other internal members
typedef udMutableString<64 - sizeof(udSlice<char>)> udMutableString64;
typedef udMutableString<128 - sizeof(udSlice<char>)> udMutableString128;
typedef udMutableString<256 - sizeof(udSlice<char>)> udMutableString256;


// reference-counted allocated string, used to retain ownership of arbitrary length strings
// reference counting allows allocations to be shared between multiple owners
// useful in situations where std::string would usually be found, but without the endless allocation and copying or linkage problems
struct udSharedString : public udSharedSlice<const char>
{
  // constructors
  udSharedString();
  udSharedString(udSharedString &&rval);
  udSharedString(const udSharedString &val);
  udSharedString(udSharedSlice<const char> &&rval);
  udSharedString(const udSharedSlice<const char> &rcstr);
  template <typename U>
  udSharedString(U *ptr, size_t length);
  template <typename U>
  udSharedString(udSlice<U> slice);
  udSharedString(const char *pString);

  // construction
  template<typename... Args> static udSharedString concat(const Args&... args);
  template<typename... Args> static udSharedString format(udString format, const Args&... args);
  static udSharedString sprintf(const char *pFormat, ...);

  // assignment
  udSharedString& operator =(const udSharedSlice<const char> &rh);
  udSharedString& operator =(const udSharedString &rval);
  udSharedString& operator =(udSharedString &&rval);
  udSharedString& operator =(udSharedSlice<const char> &&rval);
  template <typename U>
  udSharedString& operator =(udSlice<U> rh);
  udSharedString& operator =(const char *pString);

  // contents
  udSharedString slice(ptrdiff_t first, ptrdiff_t last) const;

  // comparison
  bool eq(udString rh) const                                                            { return ((udString*)this)->eq(rh); }
  bool eqIC(udString rh) const                                                          { return ((udString*)this)->eqIC(rh); }
  bool beginsWith(udString rh) const                                                    { return ((udString*)this)->beginsWith(rh); }
  bool beginsWithIC(udString rh) const                                                  { return ((udString*)this)->beginsWithIC(rh); }
  bool endsWith(udString rh) const                                                      { return ((udString*)this)->endsWith(rh); }
  bool endsWithIC(udString rh) const                                                    { return ((udString*)this)->endsWithIC(rh); }

  ptrdiff_t cmp(udString rh) const                                                      { return ((udString*)this)->cmp(rh); }
  ptrdiff_t cmpIC(udString rh) const                                                    { return ((udString*)this)->cmpIC(rh); }

  // c-string compatibility
  char* toStringz(char *pBuffer, size_t bufferLen) const                                { return ((udString*)this)->toStringz(pBuffer, bufferLen); }
  udCString toStringz() const;

  // transformation
  template<typename... Args> udSharedString append(const Args&... args) const           { return concat(*this, args...); }

  udSharedString asUpper() const;
  udSharedString asLower() const;

  // useful functions
  size_t findFirstIC(udString s) const                                                  { return ((udString*)this)->findFirstIC(s); }
  size_t findLastIC(udString s) const                                                   { return ((udString*)this)->findLastIC(s); }

  udSharedString getLeftAtFirstIC(udString s, bool bInclusive = false) const;
  udSharedString getLeftAtLastIC(udString s, bool bInclusive = false) const;
  udSharedString getRightAtFirstIC(udString s, bool bInclusive = true) const;
  udSharedString getRightAtLastIC(udString s, bool bInclusive = true) const;

  udSharedString trim(bool front = true, bool back = true) const;

  template<bool skipEmptyTokens = true>
  udSharedString popToken(udString delimiters = " \t\r\n");
  template<bool skipEmptyTokens = true>
  udSlice<udSharedString> tokenise(udSlice<udSharedString> tokens, udString delimiters = " \t\r\n") const;

  int64_t parseInt(bool bDetectBase = true, int base = 10) const                        { return ((udString*)this)->parseInt(bDetectBase, base); }
  double parseFloat() const                                                             { return ((udString*)this)->parseFloat(); }

  uint32_t hash(uint32_t hash = 0) const                                                { return ((udString*)this)->hash(hash); }

private:
  udSharedString(const char *ptr, size_t length, udRC *rc);

  static udSharedString concatInternal(udSlice<ud_internal::VarArg> args);
  static udSharedString formatInternal(udString format, udSlice<ud_internal::VarArg> args);
};

// unit tests
udResult udString_Test();

#include "udstring.inl"

#endif

#endif // _UD_STRING
