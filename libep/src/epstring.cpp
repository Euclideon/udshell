#include "ep/cpp/platform.h"

#include <stdio.h>
#include <math.h>

#if defined(EP_COMPILER_VISUALC)
// _CRT_SECURE_NO_WARNINGS warning (vsprintf_s)
# pragma warning(disable: 4996)
#endif

extern "C" {

epSharedString epSharedString_Create(const char *pCString)
{
  epSharedString s;
  epConstruct(&s) ep::SharedString(pCString);
  return s;
}

size_t epSharedString_Acquire(const epSharedString *pSS)
{
  if (!pSS || !pSS->ptr)
    return 0;
  ep::internal::SliceHeader *pH = ep::internal::GetSliceHeader(pSS->ptr);
  return ++pH->refCount;
}
size_t epSharedString_Release(const epSharedString *pSS)
{
  if (!pSS || !pSS->ptr)
    return 0;
  ep::internal::SliceHeader *pH = ep::internal::GetSliceHeader(pSS->ptr);
  if(pH->refCount > 1)
    return --pH->refCount;
  ep::internal::SliceFree(pSS->ptr);
  ((epSharedString*)pSS)->ptr = nullptr;
  ((epSharedString*)pSS)->length = 0;
  return 0;
}


// 1 = alpha, 2 = numeric, 4 = white, 8 = newline
const char s_epCharDetails[256] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 8, 0, 0, 8, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

} // extern "C"


namespace ep {
namespace internal {

int epvscprintf(const char *format, va_list args)
{
  va_list argsCopy;
  va_copy(argsCopy, args);

#if defined(EP_NACL)
  return vsprintf(nullptr, format, args);
#elif defined(EP_COMPILER_VISUALC)
  return _vscprintf(format, argsCopy);
#else
  return vsnprintf(nullptr, 0, format, argsCopy);
#endif
}
int epvsnprintf(char *s, size_t count, const char *format, va_list args)
{
#if defined(EP_NACL)
  return vsprintf(s, format, args);
#elif defined(EP_COMPILER_VISUALC)
  return vsnprintf_s(s, count, count, format, args);
#else
  return vsnprintf(s, count, format, args);
#endif
}

size_t getLength(Slice<VarArg> args)
{
  size_t len = 0;
  for (auto &a : args)
    len += a.GetStringLength();
  return len;
}
Slice<char> concatenate(Slice<char> buffer, Slice<VarArg> args)
{
  size_t len = 0;
  for (auto &a : args)
    len += a.GetString(buffer.slice(len, buffer.length));
  return buffer.slice(0, len);
}

ptrdiff_t parseFormat(String &format, Slice<char> &buffer, Slice<VarArg> args);

int64_t parseInt(String &format, const VarArg *pArgs)
{
  if (!format)
    return -1; // number expected!

  if (format.front() == '*')
  {
    format.popFront();
    int64_t i = parseInt(format, pArgs);
    if (i < 0)
      return i;
    if (!pArgs[i].HasIntify())
    {
      EPASSERT(false, "Argument can not be interpreted as an integer!");
      return -1;
    }
    return pArgs[i].GetInt();
  }

  if (!epIsNumeric(format.front()))
    return -1; // no number!

  size_t i = 0;
  while (format && epIsNumeric(format.front()))
    i = i*10 + (format.popFront() - '0');
  return i;
}

Slice<char> format(String format, Slice<char> buffer, Slice<VarArg> args)
{
  char *pBuffer = buffer.ptr;
  size_t length = 0;

  while (format)
  {
    if (format.length > 1 && format.front() == '\\')
    {
      format.popFront();
      goto write_char;
    }
    else if (format.front() == '{')
    {
      ptrdiff_t len = parseFormat(format, buffer, args);
      if (len < 0)
      {
        EPASSERT(false, "Bad format string!");
        return nullptr;
      }
      length += len;
    }
//    else if (0) // TODO: handle ANSI codes for colour and shiz...?
//    {
//    }
    else
    {
write_char:
      char c = format.popFront();
      if (buffer.ptr)
        buffer.popFront() = c;
      ++length;
    }
  }
  return Slice<char>(pBuffer, length);
}

ptrdiff_t parseFormat(String &format, Slice<char> &buffer, Slice<VarArg> args)
{
  if (format.popFront() != '{')
  {
    EPASSERT(false, "Not a format string!");
    return -1;
  }

  format = format.trim<true, false>();
  if (!format)
    return -1;

  // check for indirection
  String immediate = nullptr;
  bool bIndirect = false;
  int64_t arg = 0;
  if (format.front() == '\'')
  {
    format.popFront();
    immediate.ptr = format.ptr;
    while (format && format.front() != '\'')
      format.popFront();
    if (!format)
      return -1;
    immediate.length = format.ptr - immediate.ptr;
    format.popFront();
  }
  else
  {
    if (format.front() == '@')
    {
      bIndirect = true;
      format.popFront();
    }

    // get the arg index
    arg = parseInt(format, args.ptr);
    if (arg < 0)
    {
      EPASSERT(false, "Invalid format string: Number expected!");
      return -1;
    }
    if (arg >= (int64_t)args.length)
    {
      EPASSERT(false, "Format string references invalid parameter!");
      return -1;
    }
  }

  format = format.trim<true, false>();
  if (!format)
    return -1;

  // get the format string (if present)
  String formatSpec;
  if (format.front() == ',')
  {
    format.popFront();
    formatSpec.ptr = format.ptr;
    while (format && format.front() != '}')
      format.popFront();
    if (!format)
      return -1;
    formatSpec.length = format.ptr - formatSpec.ptr;
    formatSpec = formatSpec.trim();
  }

  // expect terminating '}'
  if (format.popFront() != '}')
  {
    EPASSERT(false, "Invalid format string!");
    return -1;
  }

  // check for universal format strings
  MutableString64 indirectFormatSpec;
  if (formatSpec)
  {
    // indrect formatting allows to take the format string from another parameter
    while (formatSpec && (formatSpec.front() == '?' || formatSpec.front() == '!' || formatSpec.front() == '@'))
    {
      char token = formatSpec.popFront();

      int64_t index = parseInt(formatSpec, args.ptr);
      if (index < 0)
      {
        EPASSERT(false, "Invalid format string: Number expected!");
        return -1;
      }
      if ((size_t)index >= args.length)
      {
        EPASSERT(false, "Format indirection references invalid parameter!");
        return -1;
      }
      size_t i = (size_t)index;

      if (token == '?' || token == '!')
      {
        if (!args[i].HasIntify())
        {
          EPASSERT(false, "Argument can not be interpreted as an integer!");
          return -1;
        }
        int64_t condition = args[i].GetInt();
        if ((token == '?' && !condition) || (token == '!' && condition))
          return 0;
      }
      else if (token == '@')
      {
        ptrdiff_t formatLen = args[i].GetStringLength();
        indirectFormatSpec.reserve((size_t)formatLen);
        args[i].GetString(indirectFormatSpec.getBuffer());
        formatSpec = String(indirectFormatSpec.ptr, formatLen);
      }
    }
  }

  size_t len;
  if (immediate.ptr)
  {
    len = epStringify(buffer, formatSpec, immediate, args.ptr);
  }
  else if (bIndirect)
  {
    // interpret the arg as an indirect format string
    ptrdiff_t bytes = args[(size_t)arg].GetStringLength(formatSpec, args.ptr);
    MutableString128 indirectFormat(Reserve, bytes);
    args[(size_t)arg].GetString(indirectFormat.getBuffer(), formatSpec, args.ptr);
    len = internal::format(String(indirectFormat.ptr, bytes), buffer, args).length;
  }
  else
  {
    // append the arg
    len = args[(size_t)arg].GetString(buffer, formatSpec, args.ptr);
  }

  if (buffer.ptr)
    buffer.pop(len);
  return len;
}

size_t urlDecode(Slice<char> outSlice, String inStr)
{
  size_t alloc = inStr.length + 1;
  const char *string = inStr.ptr;

  size_t strindex = 0;
  while (--alloc > 0)
  {
    unsigned char in = *string;
    if (in == '+')
    {
      in = ' ';
    }
    else if ((in == '%') && (alloc > 2) && epIsHex((unsigned char)string[1]) && epIsHex((unsigned char)string[2]))
    {
      // this is two hexadecimal digits following a '%'
      char hexstr[3] = { string[1], string[2], 0 };
      in = (unsigned char)strtoul(hexstr, nullptr, 16);

      string += 2;
      alloc -= 2;
    }

    outSlice[strindex++] = in;
    string++;
  }

  return strindex;
}

} // namespace internal

SharedString SharedString::sprintf(const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);

  size_t len = internal::epvscprintf(pFormat, args) + 1;
  MutableString<0> r(Reserve, len);
  r.length = internal::epvsnprintf(r.ptr, len, pFormat, args);

  va_end(args);

  return std::move(r);
}

SharedString SharedString::concatInternal(Slice<VarArg> args)
{
  size_t len = internal::getLength(args);

  MutableString<0> r(Reserve, len+1);

  r.length = internal::concatenate(r.getBuffer(), args).length;
  r.ptr[r.length] = 0;

  return std::move(r);
}
SharedString SharedString::formatInternal(String format, Slice<VarArg> args)
{
  size_t len = internal::format(format, nullptr, args).length;

  MutableString<0> r(Reserve, len+1);

  r.length = internal::format(format, r.getBuffer(), args).length;
  r.ptr[r.length] = 0;

  return std::move(r);
}

template<typename C>
int64_t BaseString<C>::parseInt(bool bDetectBase, int base) const
{
  BaseString<C> s = trim<true, false>();
  if (s.length == 0)
    return 0; // this isn't really right!

  int number = 0;

  if (base == 16 || (bDetectBase && (s.eqIC("0x") || s.eq("$"))))
  {
    // hex number
    if (s.eqIC("0x"))
      s.pop(2);
    else if (s.eq("$"))
      s.pop(1);

    while (s.length > 0)
    {
      C digit = s.popFront();
      if (!epIsHex(digit))
        return number;
      number <<= 4;
      number += epIsNumeric(digit) ? digit - '0' : (digit|0x20) - 'a' + 10;
    }
  }
  else if (base == 2 || (bDetectBase && s.eqIC("b")))
  {
    if (s.eqIC("b"))
      s.pop(1);

    while (s.length > 0 && (s.ptr[0] == '0' || s.ptr[0] == '1'))
    {
      number <<= 1;
      number |= s.ptr[0] - '0';
    }
  }
  else if (base == 10)
  {
    // decimal number
    bool neg = false;
    if (s.ptr[0] == '-' || s.ptr[0] == '+')
    {
      neg = s.ptr[0] == '-';
      s.pop(1);
    }

    while (s.length > 0)
    {
      C c = s.popFront();
      if (!epIsNumeric(c))
        break;
      number = number*10 + c - '0';
    }
    if (neg)
      number = -number;
  }

  return number;
}

template<typename C>
double BaseString<C>::parseFloat() const
{
  BaseString<C> s = trim<true, false>();
  if (s.length == 0)
    return 0; // this isn't really right!

  int64_t number = 0;
  int64_t frac = 0;
  size_t fracSize = 0;
  int16_t exponent = 0;
  int8_t expSign = 1;

  // floating poiont number
  bool neg = false;
  if (s.ptr[0] == '-' || s.ptr[0] == '+')
  {
    neg = s.ptr[0] == '-';
    s.popFront();
  }

  bool bHasDot = false;
  bool bHasExp = false;

  while (s.length > 0)
  {
    C digit = s.popFront();
    if (!epIsNumeric(digit) && (bHasDot || digit != '.') && (bHasExp || (digit != 'e' && digit != 'E')))
      break;
    if (digit == '.')
      bHasDot = true;
    else if (digit == 'e' || digit == 'E')
    {
      if(s.length > 0 && (s[0] == '-' || s[0] == '+'))
      {
        if (s[0] == '-')
          expSign = -1;

        s.popFront();
      }
    }
    else
    {
      if (!bHasDot)
        number = number*10 + digit - '0';
      else if (!bHasExp)
      {
        frac = frac*10 + digit - '0';
        fracSize++;
      }
      else
        exponent = exponent * 10 + digit - '0';
    }
  }

  if (neg)
  {
    number = -number;
    frac = -frac;
  }

  return ((double)number + (double)frac/pow(10.0, (double)fracSize)) * pow(10.0, (double)(expSign * exponent));
}

template int64_t BaseString<char>::parseInt(bool, int) const;
template double BaseString<char>::parseFloat() const;
//template int64_t BaseString<char16_t>::parseInt(bool, int) const;
//template double BaseString<char16_t>::parseFloat() const;
//template int64_t BaseString<char32_t>::parseInt(bool, int) const;
//template double BaseString<char32_t>::parseFloat() const;

} // ep

ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String epUnusedParam(format), nullptr_t, const ep::VarArg *epUnusedParam(pArgs))
{
  if (buffer.ptr)
    ep::String("null", 4).copyTo(buffer);
  return 4;
}

template<typename C>
ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String format, ep::BaseString<C> s, const ep::VarArg *pArgs)
{
  // parse format string
  bool rightJustify = false;
  int64_t minimumLen = 0;
  while (format)
  {
    if (format[0] == '-')
    {
      rightJustify = true;
      format.pop(1);
    }
    else if (format[0] == '*')
    {
      int64_t i = format.slice(1, format.length).parseInt();
      EPASSERT(pArgs[i].HasIntify(), "Argument can not be interpreted as an integer!");
      minimumLen = pArgs[i].GetInt();
      EPASSERT(minimumLen >= 0, "Invalid string length!");
      break;
    }
    else
    {
      // TODO: error here? invalid format string?
      format.pop(1);
    }
  }

  // calculate UTF8 source len
  size_t secLen = 0;
  if (sizeof(C) == sizeof(char)) // type of buffer
    secLen = s.length;
  else
  {
    for (size_t i = 0; i<s.length; )
    {
      size_t l = 0;
      secLen += epUTF8SequenceLength(s.ptr + i, &l);
      i += l;
    }
  }

  // calculate string length
  bool needPadding = (size_t)minimumLen > s.length;
  size_t length = needPadding ? (size_t)minimumLen : s.length;

  // if we're only counting
  if (!buffer.ptr)
    return length;

  // if the buffer is too small
  if (buffer.length < length)
    return buffer.length - length;

  char *pBuffer = buffer.ptr;

  // do left padding
  if (rightJustify && needPadding)
  {
    memset(pBuffer, ' ', length - s.length);
    pBuffer += length - s.length;
  }

  // copy string
  if (sizeof(C) == sizeof(char)) // type of buffer
    memcpy(pBuffer, s.ptr, sizeof(C)*s.length);
  else
  {
    size_t offset = 0;
    while (s)
    {
      char32_t c = s.popFrontChar();
      offset += epUTFEncode(c, pBuffer + offset);
    }
  }

  // do right padding
  if (!rightJustify && needPadding)
    memset(pBuffer + secLen, ' ', length - secLen);

  return length;
}
template ptrdiff_t epStringify<char>(ep::Slice<char> buffer, ep::String format, ep::BaseString<char> s, const ep::VarArg *pArgs);
template ptrdiff_t epStringify<char16_t>(ep::Slice<char> buffer, ep::String format, ep::BaseString<char16_t> s, const ep::VarArg *pArgs);
template ptrdiff_t epStringify<char32_t>(ep::Slice<char> buffer, ep::String format, ep::BaseString<char32_t> s, const ep::VarArg *pArgs);

ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String epUnusedParam(format), bool b, const ep::VarArg *epUnusedParam(pArgs))
{
  if (b == true)
  {
    if (buffer.ptr)
    {
      if (buffer.length < 4)
        return buffer.length - 4;
      ep::String("true", 4).copyTo(buffer);
    }
    return 4;
  }
  else
  {
    if (buffer.ptr)
    {
      if (buffer.length < 5)
        return buffer.length - 5;
      ep::String("false", 5).copyTo(buffer);
    }
    return 5;
  }
}

ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String epUnusedParam(format), int64_t i, const ep::VarArg *epUnusedParam(pArgs))
{
  // TODO: what formats are interesting for ints?

  // TODO: this will crash if buffer runs out of space!
  size_t start;
  size_t len;
  if (i < 0)
  {
    if (buffer.ptr)
      buffer.ptr[0] = '-';
    start = len = 1;
    do
    {
      if (buffer.ptr)
        buffer.ptr[len++] = '0' - i%10;
      else
        ++len;
    } while ((i /= 10) != 0);
  }
  else
  {
    start = len = 0;
    do
    {
      if (buffer.ptr)
        buffer.ptr[len++] = '0' + i%10;
      else
        ++len;
    } while ((i /= 10) != 0);
  }

  if (buffer.ptr)
  {
    // number is written little endian, so we need to reverse it
    size_t end = len-1;
    while (start < end)
    {
      char t = buffer.ptr[end];
      buffer.ptr[end--] = buffer.ptr[start];
      buffer.ptr[start++] = t;
    }
  }
  return len;
}

ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String epUnusedParam(format), uint64_t i, const ep::VarArg *epUnusedParam(pArgs))
{
  // TODO: what formats are interesting for ints?

  size_t len = 0;
  do
  {
    if (buffer.ptr)
    {
      if (buffer.length <= len)
        return 0;

      buffer.ptr[len++] = '0' + i % 10;
    }
    else
      ++len;
  } while ((i /= 10) != 0);

  if (buffer.ptr)
  {
    // number is written little endian, so we need to reverse it
    size_t start = 0;
    size_t end = len - 1;
    while (start < end)
    {
      char t = buffer.ptr[end];
      buffer.ptr[end--] = buffer.ptr[start];
      buffer.ptr[start++] = t;
    }
  }
  return len;
}
ptrdiff_t epStringify(ep::Slice<char> buffer, ep::String epUnusedParam(format), double f, const ep::VarArg *epUnusedParam(pArgs))
{
  const char *defaultFormat = "%.17g";
  const char *pFormat = defaultFormat;

  size_t len = 0;

  // TODO: what formats are interesting for floats? (support format param)

  if (!buffer.ptr)
  {
    #if defined(EP_NACL)
      len = sprintf(nullptr, pFormat, f);
    #else
      len = snprintf(nullptr, 0, pFormat, f);
    #endif
  }
  else
  {
    #if defined(EP_NACL)
      len = sprintf(buffer.ptr, pFormat, f);
    #else
      len = snprintf(buffer.ptr, buffer.length, pFormat, f);
    #endif
  }
  return len;
}

#include "ep/cpp/variant.h"
