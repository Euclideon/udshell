#if !defined(_EP_STRING_H)
#define _EP_STRING_H

extern const char s_epCharDetails[256];

#define epIsNewline(c) ((c) < 256 && (s_epCharDetails[c] & 8))
#define epIsWhitespace(c) ((c) < 256 && (s_epCharDetails[c] & 0xC))
#define epIsAlpha(c) ((c) < 256 && (s_epCharDetails[c] & 1))
#define epIsNumeric(c) ((c) < 256 && (s_epCharDetails[c] & 2))
#define epIsAlphaNumeric(c) ((c) < 256 && (s_epCharDetails[c] & 3))
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

#endif // _EP_STRING_H
