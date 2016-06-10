namespace ep {

inline size_t UTFEncode(char32_t c, char *pUTF8)
{
  if (c < 0x80)
  {
    pUTF8[0] = (char)c;
    return 1;
  }
  else if (c < 0x800)
  {
    pUTF8[1] = c & 0x3f; c >>= 6;
    pUTF8[0] = (c & 0x1f) | 0xc0;
    return 2;
  }
  else if (c < 0x10000)
  {
    pUTF8[2] = c & 0x3f; c >>= 6;
    pUTF8[1] = c & 0x3f; c >>= 6;
    pUTF8[0] = (c & 0x0f) | 0xe0;
    return 3;
  }
  else // NOTE: unicode was restricted to 20 bits, so we will stop here. the upper bits of a char32_t are truncated.
  {
    pUTF8[3] = c & 0x3f; c >>= 6;
    pUTF8[2] = c & 0x3f; c >>= 6;
    pUTF8[1] = c & 0x3f; c >>= 6;
    pUTF8[0] = (c & 0x07) | 0xf0;
    return 4;
  }
}
inline size_t UTFEncode(char32_t c, char16_t *pUTF16)
{
  if (c < 0x10000)
  {
    pUTF16[0] = (char16_t)c;
    return 1;
  }
  else
  {
    c -= 0x10000;
    pUTF16[0] = (char16_t)(0xD800 | (c >> 10));
    pUTF16[1] = (char16_t)(0xDC00 | (c & 0x3FF));
    return 2;
  }
}
inline size_t UTFEncode(char32_t c, char32_t *pUTF32)
{
  *pUTF32 = c;
  return 1;
}

inline size_t UTFDecode(const char *pUTF8, char32_t *pC)
{
  if (pUTF8[0] < 128)
  {
    *pC = pUTF8[0];
    return 1;
  }
  else if ((pUTF8[0] & 0xE0) == 0xC0)
  {
    *pC = ((char32_t)(pUTF8[0] & 0x1F) << 6) | (pUTF8[1] & 0x3F);
    return 2;
  }
  else if ((pUTF8[0] & 0xF0) == 0xE0)
  {
    *pC = ((char32_t)(pUTF8[0] & 0x0F) << 12) | ((char32_t)(pUTF8[1] & 0x3F) << 6) | (pUTF8[2] & 0x3F);
    return 3;
  }
  else
  {
    *pC = ((char32_t)(pUTF8[0] & 0x07) << 18) | ((char32_t)(pUTF8[1] & 0x3F) << 12) | ((char32_t)(pUTF8[2] & 0x3F) << 6) | (pUTF8[3] & 0x3F);
    return 4;
  }
}
inline size_t UTFDecode(const char16_t *pUTF16, char32_t *pC)
{
  if (pUTF16[0] >= 0xD800 && (pUTF16[0] & 0xFC00) == 0xD800)
  {
    *pC = ((char32_t)(pUTF16[0] & 0x3FF) << 10) | (pUTF16[1] & 0x3FF);
    return 2;
  }
  else
  {
    *pC = pUTF16[0];
    return 1;
  }
}
inline size_t UTFDecode(const char32_t *pUTF32, char32_t *pC)
{
  *pC = *pUTF32;
  return 1;
}

template<>
inline size_t UTFSequenceLength<char>(char32_t c)
{
  if (c < 0x80)
    return 1;
  else if (c < 0x800)
    return 2;
  else if (c < 0x10000)
    return 3;
  return 4;
  /*
    // NOTE: unicode was restricted to 20 bits, so we will stop here.
    else if(c < 0x200000)
      return 4;
    else if (c < 0x4000000)
      return 5;
    return 6;
  */
}
template<>
inline size_t UTFSequenceLength<char16_t>(char32_t c)
{
  return c < 0x10000 ? 1 : 2;
}
template<>
inline size_t UTFSequenceLength<char32_t>(char32_t)
{
  return 1;
}

inline size_t UTFSequenceLength(const char *pUTF8)
{
  if (pUTF8[0] < 128)
    return 1;
  else if ((pUTF8[0] & 0xE0) == 0xC0)
    return 2;
  else if ((pUTF8[0] & 0xF0) == 0xE0)
    return 3;
  else
    return 4;
}
inline size_t UTFSequenceLength(const char16_t *pUTF16)
{
  if (pUTF16[0] >= 0xD800 && (pUTF16[0] & 0xFC00) == 0xD800)
    return 2;
  else
    return 1;
}
inline size_t UTFSequenceLength(const char32_t*)
{
  return 1;
}

// get the UTF8 sequence length for an encoded character
inline size_t UTF8SequenceLength(const char *pUTF8, size_t *pSrcLen)
{
  size_t l = UTFSequenceLength(pUTF8);
  if (pSrcLen)
    *pSrcLen = l;
  return l;
}
inline size_t UTF8SequenceLength(const char16_t *pUTF16, size_t *pSrcLen)
{
  char32_t c;
  size_t l = UTFDecode(pUTF16, &c);
  if (pSrcLen)
    *pSrcLen = l;
  return UTFSequenceLength<char>(c);
}
inline size_t UTF8SequenceLength(const char32_t *pUTF32, size_t *pSrcLen)
{
  if (pSrcLen)
    *pSrcLen = 1;
  return UTFSequenceLength<char>(pUTF32[0]);
}

} // namespace ep
