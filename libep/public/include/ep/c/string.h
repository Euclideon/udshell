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

#define epToLower(c) (isAlpha(c) ? (c)|0x20 : (c))
#define epToUpper(c) (isAlpha(c) ? (c)&~0x20 : (c))

// C compilers just define String as a simple struct
struct epString
{
  size_t length;
  const char *ptr;
};

// TODO: blah blah

#if defined(__cplusplus)
}
#endif

#endif // _EP_STRING_H
