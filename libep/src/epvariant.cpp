
#include "ep/cpp/variant.h"
#include "ep/cpp/component.h"

extern "C" {

void epVariant_Release(epVariant v)
{
  // Note: Variant's destructor will clean our instance up
  Variant t;
  (epVariant&)t = v;
}

int epVariant_IsNull(epVariant v)
{
  return v.t == epVT_Null || (v.t == epVT_String && v.length == 0) || (v.t == epVT_Component && v.p == NULL);
}
char epVariant_AsBool(epVariant v)
{
  switch ((epVariantType)v.t)
  {
    case epVT_Null:
      return false;
    case epVT_Bool:
      return v.b;
    case epVT_Int:
      return !!v.i;
    case epVT_Float:
      return v.f != 0;
    case epVT_String:
    {
      String str(v.s, v.length);
      if (str.eqIC("true"))
        return true;
      else if (str.eqIC("false"))
        return false;
      return !str.empty();
    }
    default:
      EPASSERT(v.t == epVT_Bool, "Wrong type!");
      return false;
  }
}
int64_t epVariant_AsInt(epVariant v)
{
  switch ((epVariantType)v.t)
  {
    case epVT_Null:
      return 0;
    case epVT_Bool:
      return v.b ? 1 : 0;
    case epVT_Int:
      return v.i;
    case epVT_Float:
      return (int64_t)v.f;
    case epVT_String:
      return String(v.s, v.length).parseInt();
    default:
      EPASSERT(v.t == epVT_Int, "Wrong type!");
      return 0;
  }
}
double epVariant_AsFloat(epVariant v)
{
  switch ((epVariantType)v.t)
  {
    case epVT_Null:
      return 0.0;
    case epVT_Bool:
      return v.b ? 1.0 : 0.0;
    case epVT_Int:
      return (double)v.i;
    case epVT_Float:
      return v.f;
    case epVT_String:
      return String(v.s, v.length).parseFloat();
    default:
      EPASSERT(v.t == epVT_Float, "Wrong type!");
      return 0.0;
  }
}
epComponent* epVariant_AsComponent(epVariant v)
{
  switch ((epVariantType)v.t)
  {
    case epVT_Null:
      return nullptr;
    case epVT_Component:
      if (v.p)
        ++((epComponent*)v.p)->refCount;
      return (epComponent*)v.p;
    default:
      EPASSERT(v.t == epVT_Component, "Wrong type!");
      return nullptr;
  }
}
//char epVariant_GetDelegate(epVariantHandle v) {}
epString epVariant_AsString(epVariant v)
{
  // TODO: it would be nice to be able to string-ify other types
  // ...but we don't have any output buffer
  epString s = { 0, nullptr };
  switch ((epVariantType)v.t)
  {
    case epVT_Null:
      break;
    case epVT_String:
      s.length = v.length;
      s.ptr = v.s;
      break;
    case epVT_Component:
      return epComponent_GetUID((epComponent*)v.p);
    default:
      EPASSERT(v.t == epVT_String, "Wrong type!");
  }
  return s;
}

} // extern "C"


ptrdiff_t epStringifyVariant(Slice<char> buffer, String format, const Variant &v, const epVarArg *pArgs)
{
  switch (v.type())
  {
    case Variant::Type::Null:
      return epStringifyTemplate(buffer, format, nullptr, pArgs);
    case Variant::Type::Bool:
      return epStringifyTemplate(buffer, format, v.asBool(), pArgs);
    case Variant::Type::Int:
      return epStringifyTemplate(buffer, format, v.asInt(), pArgs);
    case Variant::Type::Float:
      return epStringifyTemplate(buffer, format, v.asFloat(), pArgs);
    case Variant::Type::Enum:
    case Variant::Type::Bitfield:
      EPASSERT(false, "TODO! Please write me!");
      return 0;
    case Variant::Type::String:
      return epStringifyTemplate(buffer, format, v.asString(), pArgs);
    case Variant::Type::Component:
      return epStringifyTemplate(buffer, format, v.asComponent(), pArgs);
    case Variant::Type::Delegate:
      return epStringifyTemplate(buffer, format, v.asDelegate(), pArgs);
    case Variant::Type::Array:
      return epStringifyTemplate(buffer, format, v.asArray(), pArgs);
    case Variant::Type::AssocArray:
      EPASSERT(false, "TODO! Please write me!");
      return 0;
  }
  return 0;
}

namespace ep {

const Variant InitParams::varNull;

// destructor
Variant::~Variant()
{
  if (ownsContent)
  {
    switch ((Type)t)
    {
      case Type::Component:
        ((ComponentRef&)p).~SharedPtr();
        break;
      case Type::Delegate:
        ((VarDelegate&)p).~VarDelegate();
        break;
      case Type::String:
        epFree((void*)s);
        break;
      case Type::Array:
        for (size_t i = 0; i < length; ++i)
          ((Variant*)p)[i].~Variant();
        epFree(p);
        break;
      case Type::AssocArray:
        for (size_t i = 0; i < length; ++i)
        {
          ((KeyValuePair*)p)[i].key.~Variant();
          ((KeyValuePair*)p)[i].value.~Variant();
        }
        epFree(p);
        break;
      default:
        break;
    }
  }
}

SharedString Variant::stringify() const
{
  switch ((Type)t)
  {
    case Type::Null:
      return "nil";
    case Type::Bool:
      return b ? "true" : "false";
    case Type::String:
      return asString();
    default:
      break;
  }
  return nullptr;
}

ptrdiff_t Variant::compare(const Variant &v) const
{
  if (t != v.t)
    return t - v.t;

  switch ((Type)t)
  {
    case Type::Null:
      return 0;
    case Type::Bool:
      return (b ? 1 : 0) - (v.b ? 1 : 0);
    case Type::Int:
      return ptrdiff_t(i - v.i);
    case Type::Float:
      return f < v.f ? -1 : (f > v.f ? 1 : 0);
    case Type::String:
      return String(s, length).cmp(String(v.s, v.length));
    case Type::Component:
      return ((String&)((epComponent*)p)->uid).cmp((String&)((epComponent*)v.p)->uid);
    default:
      return (char*)p - (char*)v.p;
  }
}

bool Variant::asBool() const
{
  switch ((Type)t)
  {
    case Type::Null:
      return false;
    case Type::Bool:
      return b ? true : false;
    case Type::Int:
      return !!i;
    case Type::Float:
      return f != 0;
    case Type::String:
    {
      String str(s, length);
      if (str.eqIC("true"))
        return true;
      else if (str.eqIC("false"))
        return false;
      return !str.empty();
    }
    default:
      EPASSERT(type() == Type::Bool, "Wrong type!");
      return false;
  }
}
int64_t Variant::asInt() const
{
  switch ((Type)t)
  {
    case Type::Null:
      return 0;
    case Type::Bool:
      return b ? 1 : 0;
    case Type::Int:
      return i;
    case Type::Float:
      return (int64_t)f;
    case Type::String:
      return String(s, length).parseInt();
    default:
      EPASSERT(type() == Type::Int, "Wrong type!");
      return 0;
  }
}
double Variant::asFloat() const
{
  switch ((Type)t)
  {
    case Type::Null:
      return 0.0;
    case Type::Bool:
      return b ? 1.0 : 0.0;
    case Type::Int:
      return (double)i;
    case Type::Float:
      return f;
    case Type::String:
      return String(s, length).parseFloat();
    default:
      EPASSERT(type() == Type::Float, "Wrong type!");
      return 0.0;
  }
}
const epEnumDesc* Variant::asEnum(size_t *pVal) const
{
  if ((Type)t == Type::Enum || (Type)t == Type::Bitfield)
  {
    *pVal = (ptrdiff_t)(length << 5) >> 5;
    return (const epEnumDesc*)p;
  }
  return nullptr;
}
ComponentRef Variant::asComponent() const
{
  switch ((Type)t)
  {
    case Type::Null:
      return ComponentRef();
    case Type::Component:
      return (ComponentRef&)p;
    default:
      EPASSERT(type() == Type::Component, "Wrong type!");
      return nullptr;
  }
}
Variant::VarDelegate Variant::asDelegate() const
{
  switch ((Type)t)
  {
    case Type::Null:
      return VarDelegate();
    case Type::Delegate:
      return (VarDelegate&)p;
    default:
      EPASSERT(type() == Type::Delegate, "Wrong type!");
      return VarDelegate();
  }
}
String Variant::asString() const
{
  // TODO: it would be nice to be able to string-ify other types
  // ...but we don't have any output buffer
  switch ((Type)t)
  {
    case Type::Null:
      return String();
    case Type::String:
      return String(s, length);
    case Type::Component:
      return ((Component*)p)->GetUID();
    default:
      EPASSERT(type() == Type::String, "Wrong type!");
      return String();
  }
}
Slice<Variant> Variant::asArray() const
{
  switch ((Type)t)
  {
    case Type::Null:
      return Slice<Variant>();
    case Type::Array:
      return Slice<Variant>((Variant*)p, length);
    default:
      EPASSERT(type() == Type::Array, "Wrong type!");
      return Slice<Variant>();
  }
}
Slice<KeyValuePair> Variant::asAssocArray() const
{
  switch ((Type)t)
  {
    case Type::Null:
      return Slice<KeyValuePair>();
    case Type::AssocArray:
      return Slice<KeyValuePair>((KeyValuePair*)p, length);
    default:
      EPASSERT(type() == Type::AssocArray, "Wrong type!");
      return Slice<KeyValuePair>();
  }
}
Slice<KeyValuePair> Variant::asAssocArraySeries() const
{
  switch ((Type)t)
  {
    case Type::Null:
      return Slice<KeyValuePair>();
    case Type::AssocArray:
      return Slice<KeyValuePair>((KeyValuePair*)p, assocArraySeriesLen());
    default:
      EPASSERT(type() == Type::AssocArray, "Wrong type!");
      return Slice<KeyValuePair>();
  }
}

size_t Variant::arrayLen() const
{
  if (is(Type::Array))
    return length;
  if (is(Type::AssocArray))
    return assocArraySeriesLen();
  return 0;
}
size_t Variant::assocArraySeriesLen() const
{
  if (!is(Type::AssocArray))
    return 0;
  size_t i = 0;
  while (i < length && ((KeyValuePair*)p)[i].key.is(Type::Int) && ((KeyValuePair*)p)[i].key.i == (int64_t)i + 1)
    ++i;
  return i;
}

Variant Variant::operator[](size_t i) const
{
  if (is(Type::Array))
  {
    EPASSERT(i < length, "Index out of range!");
    return ((Variant*)p)[i];
  }
  if (is(Type::AssocArray))
  {
    EPASSERT(i < assocArraySeriesLen(), "Index out of range!");
    return ((KeyValuePair*)p)[i].value;
  }
  return Variant(nullptr);
}
Variant Variant::operator[](String key) const
{
  if (is(Type::AssocArray))
  {
    size_t i = assocArraySeriesLen();
    for (; i<length; ++i)
    {
      Variant &k = ((KeyValuePair*)p)[i].key;
      if (!k.is(Type::String))
        continue;
      if (String(k.s, k.length).eq(key))
        return ((KeyValuePair*)p)[i].value;
    }
  }
  return Variant(nullptr);
}

Variant* Variant::allocArray(size_t len)
{
  this->~Variant();
  t = (size_t)Type::Array;
  length = len;
  ownsContent = true;
  ((Variant*&)p) = epAllocType(Variant, len, epAF_None);
  return ((Variant*)p);
}

KeyValuePair* Variant::allocAssocArray(size_t len)
{
  this->~Variant();
  t = (size_t)Type::AssocArray;
  length = len;
  ownsContent = true;
  ((KeyValuePair*&)p) = epAllocType(KeyValuePair, len, epAF_None);
  return ((KeyValuePair*)p);
}

} // namespace ep
