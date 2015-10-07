#include <stdio.h>
#include "ep/epstring.h"

#if defined(_MSC_VER)
// _CRT_SECURE_NO_WARNINGS warning (vsprintf_s)
# pragma warning(disable: 4996)
#endif

int64_t epStringify_ParseInt(epString &format, const epVarArg *pArgs)
{
  if (!format)
    return -1; // number expected!

  if (format.front() == '*')
  {
    format.popFront();
    int64_t i = epStringify_ParseInt(format, pArgs);
    if (i < 0)
      return i;
    if (!pArgs[i].HasIntify())
    {
      EPASSERT(false, "Argument can not be interpreted as an integer!");
      return -1;
    }
    return pArgs[i].GetInt();
  }

  if (!isNumeric(format.front()))
    return -1; // no number!

  size_t i = 0;
  while (format && isNumeric(format.front()))
    i = i*10 + (format.popFront() - '0');
  return i;
}

namespace ud_internal
{
  // 1 = alpha, 2 = numeric, 4 = white, 8 = newline
  const char charDetails[256] =
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

  size_t getLength(epSlice<epVarArg> args)
  {
    size_t len = 0;
    for (auto &a : args)
      len += a.GetStringLength();
    return len;
  }
  epSlice<char> concatenate(epSlice<char> buffer, epSlice<epVarArg> args)
  {
    size_t len = 0;
    for (auto &a : args)
      len += a.GetString(buffer.slice(len, buffer.length));
    return buffer.slice(0, len);
  }

  ptrdiff_t parseFormat(epString &format, epSlice<char> &buffer, epSlice<epVarArg> args);

  epSlice<char> format(epString format, epSlice<char> buffer, epSlice<epVarArg> args)
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
//      else if (0) // TODO: handle ANSI codes for colour and shiz...?
//      {
//      }
      else
      {
  write_char:
        char c = format.popFront();
        if (buffer.ptr)
          buffer.popFront() = c;
        ++length;
      }
    }
    return epSlice<char>(pBuffer, length);
  }

  ptrdiff_t parseFormat(epString &format, epSlice<char> &buffer, epSlice<epVarArg> args)
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
    epString immediate = nullptr;
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
      arg = epStringify_ParseInt(format, args.ptr);
      if (arg < 0)
      {
        EPASSERT(false, "Invalid format string: Number expected!");
        return -1;
      }
      if ((size_t)arg >= args.length)
      {
        EPASSERT(false, "Format string references invalid parameter!");
        return -1;
      }
    }

    format = format.trim<true, false>();
    if (!format)
      return -1;

    // get the format string (if present)
    epString formatSpec;
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
    epMutableString64 indirectFormatSpec;
    if (formatSpec)
    {
      // indrect formatting allows to take the format string from another parameter
      while (formatSpec && (formatSpec.front() == '?' || formatSpec.front() == '!' || formatSpec.front() == '@'))
      {
        char token = formatSpec.popFront();

        int64_t i = epStringify_ParseInt(formatSpec, args.ptr);
        if (i < 0)
        {
          EPASSERT(false, "Invalid format string: Number expected!");
          return -1;
        }
        if ((size_t)i >= args.length)
        {
          EPASSERT(false, "Format indirection references invalid parameter!");
          return -1;
        }

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
          formatSpec = epString(indirectFormatSpec.ptr, formatLen);
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
      epMutableString128 indirectFormat;
      ptrdiff_t bytes = args[arg].GetStringLength(formatSpec, args.ptr);
      indirectFormat.reserve(bytes);
      args[arg].GetString(indirectFormat.getBuffer(), formatSpec, args.ptr);
      len = ud_internal::format(epString(indirectFormat.ptr, bytes), buffer, args).length;
    }
    else
    {
      // append the arg
      len = args[arg].GetString(buffer, formatSpec, args.ptr);
    }

    if (buffer.ptr)
      buffer.pop(len);
    return len;
  }
}

ptrdiff_t epStringify(epSlice<char> buffer, epString epUnusedParam(format), nullptr_t, const epVarArg *epUnusedParam(pArgs))
{
  if (buffer.ptr)
    epString("null", 4).copyTo(buffer);
  return 4;
}

ptrdiff_t epStringify(epSlice<char> buffer, epString format, epString s, const epVarArg *pArgs)
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
  memcpy(pBuffer, s.ptr, s.length);

  // do right padding
  if (!rightJustify && needPadding)
    memset(pBuffer + s.length, ' ', length - s.length);

  return length;
}

ptrdiff_t epStringify(epSlice<char> buffer, epString epUnusedParam(format), bool b, const epVarArg *epUnusedParam(pArgs))
{
  if (b == true)
  {
    if (buffer.ptr)
    {
      if (buffer.length < 4)
        return buffer.length - 4;
      epString("true", 4).copyTo(buffer);
    }
    return 4;
  }
  else
  {
    if (buffer.ptr)
    {
      if (buffer.length < 5)
        return buffer.length - 5;
      epString("false", 5).copyTo(buffer);
    }
    return 5;
  }
}

ptrdiff_t epStringify(epSlice<char> buffer, epString epUnusedParam(format), int64_t i, const epVarArg *epUnusedParam(pArgs))
{
  // TODO: what formats are interesting for ints?

  // TODO: this will crash if buffer runs out of space!
  size_t start;
  size_t len;
  if (i < 0)
  {
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

ptrdiff_t epStringify(epSlice<char> buffer, epString epUnusedParam(format), uint64_t i, const epVarArg *epUnusedParam(pArgs))
{
  // TODO: what formats are interesting for ints?

  // TODO: this will crash if buffer runs out of space!
  size_t len = 0;
  do
  {
    if (buffer.ptr)
      buffer.ptr[len++] = '0' + i % 10;
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
ptrdiff_t epStringify(epSlice<char> buffer, epString epUnusedParam(format), double f, const epVarArg *epUnusedParam(pArgs))
{
  // TODO: what formats are interesting for floats?
  EPASSERT(false, "No fun!");
  return 0;
}

epSharedString epSharedString::sprintf(const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);

  size_t len = vsprintf(nullptr, pFormat, args) + 1;
  len = numToAlloc(len);

  epSharedString r;
  r.rc = (epRC*)udAlloc(sizeof(epRC) + len);
  r.rc->refCount = 1;
  r.rc->allocatedCount = len;
  r.ptr = ((const char*)r.rc)+sizeof(epRC);
#if defined(EP_NACL)
  r.length = vsprintf((char*)r.ptr, pFormat, args);
#else
  r.length = vsnprintf((char*)r.ptr, len, pFormat, args);
#endif

  va_end(args);

  return r;
}

epSharedString epSharedString::concatInternal(epSlice<epVarArg> args)
{
  size_t len = ud_internal::getLength(args);

  // allocate a new epSharedString
  epRC *pRC = (epRC*)udAlloc(sizeof(epRC) + sizeof(char)*(len+1));
  pRC->refCount = 0;
  pRC->allocatedCount = len;
  char *ptr = (char*)(pRC + 1);

  ud_internal::concatenate(epSlice<char>(ptr, len), args);
  ptr[len] = 0;

  return epSharedString(ptr, len, pRC);
}
epSharedString epSharedString::formatInternal(epString format, epSlice<epVarArg> args)
{
  size_t len = ud_internal::format(format, nullptr, args).length;

  // allocate a new epSharedString
  epRC *pRC = (epRC*)udAlloc(sizeof(epRC) + sizeof(char)*(len+1));
  pRC->refCount = 0;
  pRC->allocatedCount = len;
  char *ptr = (char*)(pRC + 1);

  ud_internal::format(format, epSlice<char>(ptr, len), args);
  ptr[len] = 0;

  return epSharedString(ptr, len, pRC);
}

template<typename C>
int64_t epBaseString<C>::parseInt(bool bDetectBase, int base) const
{
  epBaseString<C> s = trim<true, false>();
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
      if (!isHex(digit))
        return number;
      number <<= 4;
      number += isNumeric(digit) ? digit - '0' : (digit|0x20) - 'a' + 10;
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
      if (!isNumeric(c))
        break;
      number = number*10 + c - '0';
    }
    if (neg)
      number = -number;
  }

  return number;
}

template<typename C>
double epBaseString<C>::parseFloat() const
{
  epBaseString<C> s = trim<true, false>();
  if (s.length == 0)
    return 0; // this isn't really right!

  int64_t number = 0;
  double frac = 1;

  // floating poiont number
  bool neg = false;
  if (s.ptr[0] == '-' || s.ptr[0] == '+')
  {
    neg = s.ptr[0] == '-';
    s.popFront();
  }

  bool bHasDot = false;
  while (s.length > 0)
  {
    C digit = s.popFront();
    if (!isNumeric(digit) && (bHasDot || digit != '.'))
      break;
    if (digit == '.')
      bHasDot = true;
    else
    {
      number = number*10 + digit - '0';
      if (bHasDot)
        frac *= 0.1f;
    }
  }

  if (neg)
    number = -number;

  return (double)number * frac;
}

template int64_t epBaseString<char>::parseInt(bool, int) const;
template double epBaseString<char>::parseFloat() const;
//template int64_t epBaseString<char16_t>::parseInt(bool, int) const;
//template double epBaseString<char16_t>::parseFloat() const;
//template int64_t epBaseString<char32_t>::parseInt(bool, int) const;
//template double epBaseString<char32_t>::parseFloat() const;

udResult epSlice_Test()
{
  // usSlice<> tests
  int i[100];

  epSlice<int> i1(i, 100);        // slice from array
  epSlice<const int> ci1(i, 100); // const slice from mutable array
  ci1 = i1;                       // assign mutable to const

  ci1 == i1;                      // compare pointer AND length are equal
  ci1 != i1;                      // compare pointer OR length are not equal

  i1.eq(ci1);                     // test elements for equality

  short s[100];
  epSlice<short> s1(s, 100);

  i1.eq(s1);                      // test elements of different (compatible) types for equality (ie, int == short)

  auto slice = i1.slice(10, 20);  // 'slice' is a slice of i1 from elements [10, 20)  (exclusive: ie, [10, 19])
  slice[0];                       // element 0 from slice (ie, element 10 from i1)

  slice.empty();                  // ptr == nullptr || length == 0


  // epArray<>
  epArray<int> s_i(i1);
  epArray<const int> s_ci(s1);

  epSlice<int> s_slice = s_i.slice(1, 3); // slices of epArray are not owned; they die when the parent allocation dies


  // epSharedSlice<> tests
  epSharedSlice<int> rc_i1(i1);       // rc_i1 is an allocated, ref-counted copy of the slice i1
  epSharedSlice<const int> rc_ci1(s1);// rc_ci1 initialised from different (compatible) types (ie, short -> int, float -> double)

  // TODO: assign mutable RCSlice to const RCSlice without copy, should only bump RC

  auto rc_i2 = rc_i1;             // just inc the ref count

  auto rc_slice = rc_i1.slice(10, 20);  // slice increments the rc too

  rc_i1 = rc_slice;               // assignment between RC slices of the same array elide rc fiddling; only updates the offset/length

  i1 == rc_i2;                    // comparison of epSlice and epSharedSlice; compares ptr && length as usual

  rc_i1.eq(i1);                   // element comparison between epSlice and epSharedSlice

  for (auto i : i1)
  {
    // iterate the elements in i1
    //...
  }

  return udR_Success;
}

void receivesString(epString)
{
}
void receivesCString(const char*)
{
}

#include "ep/epvariant.h"

udResult epString_Test()
{
  // epString
  char buffer[] = "world";

  epString s1 = "hello";        // initialise from string
  epString s2(buffer, 3);       // initialise to sub-string; ie, "wor"

  s1.eq(s2);                    // strcmp() == 0
  s1.eqIC(s2);                   // case insensitive; stricmp() == 0
  s1.eq("hello");               // compare with c-string

  epSharedSlice<wchar_t> wcs(s1);   // init w_char string from c-string! yay unicode! (except we probably also want to decode utf-8...)

  wcs.eq(s1);                   // compare wide-char and ascii strings

  auto subStr = s1.slice(1, 4); // string slice; "ell"

  s2.toStringz(buffer, sizeof(buffer)); // write epString to c-string


  // epMutableString
  epMutableString<64> s_s1(s1);
  epString s_slice = s_s1.slice(1, 4); // slices of epArray are not owned; they die when the parent allocation dies

  s_s1.eqIC("HELLO");            // string comparison against string literals

  receivesString(s_s1);         // pass to functions

  s_s1.reserve(100);            // reserve a big buffer
  EPASSERT(s_s1.eq(s1), "!");   // the existing contents is preserved

  s_s1.concat(s1, "!!", epString("world"));


  // epSharedString
  epSharedString rcs1(s1);          // RC string initialised from some slice
  epSharedString rcs2("string");    // also from literal
  epSharedString rcs3(buffer, 4);   // also from c-string (and optionally a slice thereof)

  receivesString(rcs1);         // pass to functions

  epMutableString<64> ss2 = rcs2; // stack string takes copy of a epSharedString
  epSharedString rcs4 = ss2;        // rc strings take copy of stack strings too

  rcs1 == s1;                   // compare epSharedString and epString pointers

  rcs1.eqIC(s1);                 // string comparison works too between epSharedString and epString

//  epSharedString::format("Format: %s", "hello");  // create from format string

//  char temp[256];
//  fopen(rcs1.toStringz(temp, sizeof(temp)), "ro"); // write epSharedString to c-string, for passing to OS functions or C api's
                                                   // unlike c_str(), user supplies buffer (saves allocations)

  epSharedString r2 = epSharedString::concat(s1, "!!", rcs1, epString("world"), s_s1);
//  epSharedString::format("x{1}_{2}", "10", "200");

  receivesCString(r2.toStringz());

  const char *pName = "manu";
  epSharedString cc = epSharedString::concat("hello ", pName, 10);
  epSharedString fmt = epSharedString::format("{ 1 }, {2}, { 0 , hello }", "hello ", pName, 10);

  epMutableString<0> ms; ms.concat("hello ", pName, 10);
  ms.append("poop!");

  int arr[] = { 1, 2, 30 };
  ms.format("{1}, {'?',?8}{'!',!8}, {@7} {3}, {4}", "hello ", pName, 10, epSlice<int>(arr, 3), epVariant(true), "*6", 10, "!{0,@5}!", false);

  epSlice<const void> poo;
//  poo.slice(1, 3);
//  poo[2];
//  poo.alloc(10);

  return udR_Success;
}

