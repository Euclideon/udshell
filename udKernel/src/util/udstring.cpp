#include <stdio.h>
#include "udstring.h"

#if defined(_MSC_VER)
// _CRT_SECURE_NO_WARNINGS warning (vsprintf_s)
# pragma warning(disable: 4996)
#endif

// 1 = alpha, 2 = numeric, 4 = white, 8 = newline
const char s_charDetails[256] =
{
  0,0,0,0,0,0,0,0,0,4,8,0,0,8,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

udSharedString udSharedString::sprintf(const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);

  size_t len = vsprintf(nullptr, pFormat, args) + 1;
  len = numToAlloc(len);

  udSharedString r;
  r.rc = (udRC*)udAlloc(sizeof(udRC) + len);
  r.rc->refCount = 1;
  r.rc->allocatedCount = len;
  r.ptr = ((const char*)r.rc)+sizeof(udRC);
#if UDPLATFORM_NACL
  r.length = vsprintf((char*)r.ptr, pFormat, args);
#else
  r.length = vsnprintf((char*)r.ptr, len, pFormat, args);
#endif

  va_end(args);

  return r;
}

ptrdiff_t udStringify(udSlice<char> buffer, udString format, udString s)
{
  // TODO: what formats are interesting for strings?

  if (!buffer.ptr)
    return s.length;

  if (buffer.length < s.length)
    return buffer.length - s.length;
  for (size_t i = 0; i < s.length; ++i)
    buffer.ptr[i] = s.ptr[i];
  return s.length;
}

ptrdiff_t udStringify(udSlice<char> buffer, udString format, const char *s)
{
  // TODO: what formats are interesting for strings?

  if (!buffer.ptr)
    return strlen(s);

  size_t i = 0;
  for (; s[i]; ++i)
  {
    if (i == buffer.length)
      return -1;
    buffer.ptr[i] = s[i];
  }
  return i;
}

ptrdiff_t udStringify(udSlice<char> buffer, udString format, bool b)
{
  if (b == true)
  {
    if (buffer.ptr)
    {
      if (buffer.length < 4)
        return buffer.length - 4;
      udString("true", 4).copyTo(buffer);
    }
    return 4;
  }
  else
  {
    if (buffer.ptr)
    {
      if (buffer.length < 5)
        return buffer.length - 5;
      udString("false", 5).copyTo(buffer);
    }
    return 5;
  }
}

ptrdiff_t udStringify(udSlice<char> buffer, udString format, int64_t i)
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

ptrdiff_t udStringify(udSlice<char> buffer, udString format, uint64_t i)
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
ptrdiff_t udStringify(udSlice<char> buffer, udString format, double i)
{
  // TODO: what formats are interesting for floats?
  UDASSERT(false, "No fun!");
  return 0;
}

namespace ud_internal
{
  size_t getLength(udSlice<VarArg> args)
  {
    size_t len = 0;
    for (auto &a : args)
      len += a.pProxy(nullptr, nullptr, a.pArg);
    return len;
  }
  udSlice<char> concatenate(udSlice<char> buffer, udSlice<VarArg> args)
  {
    size_t len = 0;
    for (auto &a : args)
      len += a.pProxy(buffer.slice(len, buffer.length), nullptr, a.pArg);
    return buffer.slice(0, len);
  }
  udSlice<char> format(udString format, udSlice<char> buffer, udSlice<VarArg> args)
  {
    size_t offset = 0;
    char *pBuffer = buffer.ptr;

    const char *pC = format.ptr;
    const char *pEnd = format.ptr + format.length;
    while (pC < pEnd)
    {
      if (*pC == '\\' && pC[1] != 0)
      {
        // print escaped characters directly
        ++pC;
        if (pC == pEnd)
          break;
        if (pBuffer)
          pBuffer[offset] = *pC;
        ++offset;
        ++pC;
      }
      else if (*pC == '{')
      {
        ++pC;
        while (isWhitespace(*pC) && pC < pEnd)
          ++pC;
        if (pC == pEnd)
          return nullptr;

        // get the arg index
        if (!isNumeric(*pC))
        {
          UDASSERT(false, "Invalid format string!");
          return nullptr;
        }
        size_t arg = 0;
        while (isNumeric(*pC) && pC < pEnd)
          arg = arg*10 + (*pC++ - '0');
        if (arg >= args.length)
        {
          UDASSERT(false, "Format string references invalid parameter!");
          return nullptr;
        }
        while (isWhitespace(*pC) && pC < pEnd)
          ++pC;
        if (pC == pEnd)
          return nullptr;

        // get the format string (if present)
        udString format;
        if (*pC == ',')
        {
          ++pC;
          while (isWhitespace(*pC) && pC < pEnd)
            ++pC;
          if (pC == pEnd)
            return nullptr;
          format.ptr = pC;
          while (*pC != '}' && !isWhitespace(*pC) && pC < pEnd)
            pC++;
          format.length = pC - format.ptr;
        }
        while (isWhitespace(*pC) && pC < pEnd)
          ++pC;
        if (pC == pEnd)
          return nullptr;

        // expect terminating '}'
        if (*pC++ != '}')
        {
          UDASSERT(false, "Invalid format string!");
          return nullptr;
        }

        // append the arg
        VarArg &p = args[arg];
        ptrdiff_t len;
        udSlice<char> buf(pBuffer ? pBuffer + offset : nullptr, pBuffer ? buffer.length - offset : 0);
        len = p.pProxy(buf, format, p.pArg);
        offset += len;
      }
      else
      {
        if (pBuffer)
          pBuffer[offset] = *pC;
        ++offset;
        ++pC;
      }
    }
    return udSlice<char>(pBuffer, offset);
  }
}

udSharedString udSharedString::concatInternal(udSlice<ud_internal::VarArg> args)
{
  size_t len = ud_internal::getLength(args);

  // allocate a new udSharedString
  udRC *pRC = (udRC*)udAlloc(sizeof(udRC) + sizeof(char)*(len+1));
  pRC->refCount = 0;
  pRC->allocatedCount = len;
  char *ptr = (char*)(pRC + 1);

  ud_internal::concatenate(udSlice<char>(ptr, len), args);
  ptr[len] = 0;

  return udSharedString(ptr, len, pRC);
}
udSharedString udSharedString::formatInternal(udString format, udSlice<ud_internal::VarArg> args)
{
  size_t len = ud_internal::format(format, nullptr, args).length;

  // allocate a new udSharedString
  udRC *pRC = (udRC*)udAlloc(sizeof(udRC) + sizeof(char)*(len+1));
  pRC->refCount = 0;
  pRC->allocatedCount = len;
  char *ptr = (char*)(pRC + 1);

  ud_internal::format(format, udSlice<char>(ptr, len), args);
  ptr[len] = 0;

  return udSharedString(ptr, len, pRC);
}

int64_t udString::parseInt(bool bDetectBase, int base) const
{
  udString s = trim(true, false);
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
      int digit = s.popFront();
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
      unsigned char c = s.popFront();
      if (!isNumeric(c))
        break;
      number = number*10 + c - '0';
    }
    if (neg)
      number = -number;
  }

  return number;
}

double udString::parseFloat() const
{
  udString s = trim(true, false);
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
    int digit = s.popFront();
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

udResult udSlice_Test()
{
  // usSlice<> tests
  int i[100];

  udSlice<int> i1(i, 100);        // slice from array
  udSlice<const int> ci1(i, 100); // const slice from mutable array
  ci1 = i1;                       // assign mutable to const

  ci1 == i1;                      // compare pointer AND length are equal
  ci1 != i1;                      // compare pointer OR length are not equal

  i1.eq(ci1);                     // test elements for equality

  short s[100];
  udSlice<short> s1(s, 100);

  i1.eq(s1);                      // test elements of different (compatible) types for equality (ie, int == short)

  auto slice = i1.slice(10, 20);  // 'slice' is a slice of i1 from elements [10, 20)  (exclusive: ie, [10, 19])
  slice[0];                       // element 0 from slice (ie, element 10 from i1)

  slice.empty();                  // ptr == nullptr || length == 0


  // udFixedSlice<>
  udFixedSlice<int> s_i(i1);
  udFixedSlice<const int> s_ci(s1);

  udSlice<int> s_slice = s_i.slice(1, 3); // slices of udFixedSlice are not owned; they die when the parent allocation dies


  // udSharedSlice<> tests
  udSharedSlice<int> rc_i1(i1);       // rc_i1 is an allocated, ref-counted copy of the slice i1
  udSharedSlice<const int> rc_ci1(s1);// rc_ci1 initialised from different (compatible) types (ie, short -> int, float -> double)

  // TODO: assign mutable RCSlice to const RCSlice without copy, should only bump RC

  auto rc_i2 = rc_i1;             // just inc the ref count

  auto rc_slice = rc_i1.slice(10, 20);  // slice increments the rc too

  rc_i1 = rc_slice;               // assignment between RC slices of the same array elide rc fiddling; only updates the offset/length

  i1 == rc_i2;                    // comparison of udSlice and udSharedSlice; compares ptr && length as usual

  rc_i1.eq(i1);                   // element comparison between udSlice and udSharedSlice

  for (auto i : i1)
  {
    // iterate the elements in i1
    //...
  }

  return udR_Success;
}

void receivesString(udString)
{
}
void receivesCString(const char*)
{
}

udResult udString_Test()
{
  // udString
  char buffer[] = "world";

  udString s1 = "hello";        // initialise from string
  udString s2(buffer, 3);       // initialise to sub-string; ie, "wor"

  s1.eq(s2);                    // strcmp() == 0
  s1.eqIC(s2);                   // case insensitive; stricmp() == 0
  s1.eq("hello");               // compare with c-string

  udSharedSlice<wchar_t> wcs(s1);   // init w_char string from c-string! yay unicode! (except we probably also want to decode utf-8...)

  wcs.eq(s1);                   // compare wide-char and ascii strings

  auto subStr = s1.slice(1, 4); // string slice; "ell"

  s2.toStringz(buffer, sizeof(buffer)); // write udString to c-string


  // udMutableString
  udMutableString<64> s_s1(s1);
  udString s_slice = s_s1.slice(1, 4); // slices of udFixedSlice are not owned; they die when the parent allocation dies

  s_s1.eqIC("HELLO");            // string comparison against string literals

  receivesString(s_s1);         // pass to functions

  s_s1.reserve(100);            // reserve a big buffer
  UDASSERT(s_s1.eq(s1), "!");   // the existing contents is preserved

  s_s1.concat(s1, "!!", udString("world"));


  // udSharedString
  udSharedString rcs1(s1);          // RC string initialised from some slice
  udSharedString rcs2("string");    // also from literal
  udSharedString rcs3(buffer, 4);   // also from c-string (and optionally a slice thereof)

  receivesString(rcs1);         // pass to functions

  udMutableString<64> ss2 = rcs2; // stack string takes copy of a udSharedString
  udSharedString rcs4 = ss2;        // rc strings take copy of stack strings too

  rcs1 == s1;                   // compare udSharedString and udString pointers

  rcs1.eqIC(s1);                 // string comparison works too between udSharedString and udString

//  udSharedString::format("Format: %s", "hello");  // create from format string

//  char temp[256];
//  fopen(rcs1.toStringz(temp, sizeof(temp)), "ro"); // write udSharedString to c-string, for passing to OS functions or C api's
                                                   // unlike c_str(), user supplies buffer (saves allocations)

  udSharedString r2 = udSharedString::concat(s1, "!!", rcs1, udString("world"), s_s1);
//  udSharedString::format("x{1}_{2}", "10", "200");

  receivesCString(r2.toStringz());

  const char *pName = "manu";
  udSharedString cc = udSharedString::concat("hello ", pName, 10);
  udSharedString fmt = udSharedString::format("{ 1 }, {2}, { 0 , hello }", "hello ", pName, 10);

  udMutableString<0> ms; ms.concat("hello ", pName, 10);
  ms.append("poop!");
  ms.format("{1}, {2}, {0,hello}", "hello ", pName, 10);

  return udR_Success;
}

