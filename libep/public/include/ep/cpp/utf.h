#pragma once
#if !defined(_EP_UTF)
#define _EP_UTF

namespace ep {

// UTF encode a character
inline size_t utfEncode(char32_t c, char *pUTF8);
inline size_t utfEncode(char32_t c, char16_t *pUTF16);
inline size_t utfEncode(char32_t c, char32_t *pUTF32);

// decode a UTF character
inline size_t utfDecode(const char *pUTF8, char32_t *pC);
inline size_t utfDecode(const char16_t *pUTF16, char32_t *pC);
inline size_t utfDecode(const char32_t *pUTF32, char32_t *pC);

// calculate a characters UTF sequence length for the given encoding
template<typename C>
inline size_t utfSequenceLength(char32_t c);

// get the length of a UTF sequence
inline size_t utfSequenceLength(const char *pUTF8);
inline size_t utfSequenceLength(const char16_t *pUTF16);
inline size_t utfSequenceLength(const char32_t*);

// get the UTF8 sequence length for an encoded character
inline size_t utf8SequenceLength(const char *pUTF8, size_t *pSrcLen = nullptr);
inline size_t utf8SequenceLength(const char16_t *pUTF16, size_t *pSrcLen = nullptr);
inline size_t utf8SequenceLength(const char32_t *pUTF32, size_t *pSrcLen = nullptr);

} // namespace ep

#include "ep/cpp/internal/utf_inl.h"

#endif // !defined(_EP_UTF)
