#if !defined(_EP_STRING_H)
#define _EP_STRING_H

#if defined(__cplusplus)
extern "C" {
#endif

extern const char s_epCharDetails[256];

#define epIsNewline(c) ((c) < 256 && (s_epCharDetails[(size_t)c] & 8))
#define epIsWhitespace(c) ((c) < 256 && (s_epCharDetails[(size_t)c] & 0xC))
#define epIsAlpha(c) ((c) < 256 && (s_epCharDetails[(size_t)c] & 1))
#define epIsNumeric(c) ((c) < 256 && (s_epCharDetails[(size_t)c] & 2))
#define epIsAlphaNumeric(c) ((c) < 256 && (s_epCharDetails[(size_t)c] & 3))
#define epIsHex(c) (epIsAlphaNumeric(c) && ((c)|0x20) <= 'f')

#define epToLower(c) (epIsAlpha(c) ? (c)|0x20 : (c))
#define epToUpper(c) (epIsAlpha(c) ? (c)&~0x20 : (c))

// C compilers just define epString as a simple struct
struct epString
{
  size_t length;
  const char *ptr;
};

inline epString epString_Create(const char *pCString)
{
  epString s;
  s.length = strlen(pCString);
  s.ptr = pCString;
  return s;
}


// C compilers just define epSharedString as a simple struct
struct epSharedString
{
  size_t length;
  const char *ptr;
};

epSharedString epSharedString_Create(const char *pCString);
size_t epSharedString_Acquire(const epSharedString *pSS);
size_t epSharedString_Release(const epSharedString *pSS);

#if defined(__cplusplus)
}
#endif

#endif // _EP_STRING_H
