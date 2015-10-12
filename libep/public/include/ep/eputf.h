#pragma once
#if !defined(_EP_UTF)
#define _EP_UTF

// UTF encode a character
inline size_t epUTFEncode(char32_t c, char *pUTF8);
inline size_t epUTFEncode(char32_t c, char16_t *pUTF16);
inline size_t epUTFEncode(char32_t c, char32_t *pUTF32);

// decode a UTF character
inline size_t epUTFDecode(const char *pUTF8, char32_t *pC);
inline size_t epUTFDecode(const char16_t *pUTF16, char32_t *pC);
inline size_t epUTFDecode(const char32_t *pUTF32, char32_t *pC);

// calculate a characters UTF sequence length for the given encoding
template<typename C>
inline size_t epUTFSequenceLength(char32_t c);

// get the length of a UTF sequence
inline size_t epUTFSequenceLength(const char *pUTF8);
inline size_t epUTFSequenceLength(const char16_t *pUTF16);
inline size_t epUTFSequenceLength(const char32_t*);

// get the UTF8 sequence length for an encoded character
inline size_t epUTF8SequenceLength(const char *pUTF8, size_t *pSrcLen = nullptr);
inline size_t epUTF8SequenceLength(const char16_t *pUTF16, size_t *pSrcLen = nullptr);
inline size_t epUTF8SequenceLength(const char32_t *pUTF32, size_t *pSrcLen = nullptr);


#include "ep/eputf.inl"

#endif // !defined(_EP_UTF)
