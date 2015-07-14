
#include "udVariant.h"
#include "udComponent.h"

udRCString udVariant::stringify() const
{
  switch (t)
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

bool udVariant::asBool() const
{
  switch (t)
  {
  case Type::Null:
    return false;
  case Type::Bool:
    return b;
  case Type::Int:
    return !!i;
  case Type::Float:
    return f != 0;
  case Type::String:
  {
    udString s(s, length);
    if (s.eqi("true"))
      return true;
    else if (s.eqi("false"))
      return false;
    return !s.empty();
  }
  default:
    UDASSERT(type() == Type::Bool, "Wrong type!");
    return false;
  }
}
int64_t udVariant::asInt() const
{
  switch (t)
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
    return udString(s, length).parseInt();
  default:
    UDASSERT(type() == Type::Int, "Wrong type!");
    return 0;
  }
}
double udVariant::asFloat() const
{
  switch (t)
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
    return udString(s, length).parseFloat();
  default:
    UDASSERT(type() == Type::Float, "Wrong type!");
    return 0.0;
  }
}
udComponentRef udVariant::asComponent() const
{
  switch (t)
  {
  case Type::Component:
    return udComponentRef(c);
  default:
    UDASSERT(type() == Type::Component, "Wrong type!");
    return udComponentRef();
  }
}
udString udVariant::asString() const
{
  // TODO: it would be nice to be able to string-ify other types
  // ...but we don't have any output buffer
  switch (t)
  {
  case Type::Null:
    return udString();
  case Type::String:
    return udString(s, length);
  case Type::Component:
  {
    udString s;
    udResult r = c->getUid(s);
    return r == udR_Success ? s : nullptr;
  }
  default:
    UDASSERT(type() == Type::String, "Wrong type!");
    return udString();
  }
}
udSlice<udVariant> udVariant::asArray() const
{
  switch (t)
  {
  case Type::Null:
    return udSlice<udVariant>();
  case Type::Array:
    return udSlice<udVariant>(a, length);
  default:
    UDASSERT(type() == Type::Array, "Wrong type!");
    return udSlice<udVariant>();
  }
}
udSlice<udKeyValuePair> udVariant::asAssocArray() const
{
  switch (t)
  {
  case Type::Null:
    return udSlice<udKeyValuePair>();
  case Type::AssocArray:
    return udSlice<udKeyValuePair>(aa, length);
  default:
    UDASSERT(type() == Type::AssocArray, "Wrong type!");
    return udSlice<udKeyValuePair>();
  }
}
udSlice<udKeyValuePair> udVariant::asAssocArraySeries() const
{
  switch (t)
  {
  case Type::Null:
    return udSlice<udKeyValuePair>();
  case Type::AssocArray:
  {
    size_t i = 0;
    while (i < length && aa[i].key.t == Type::Int && aa[i].key.i == i + 1)
      ++i;
    return udSlice<udKeyValuePair>(aa, i);
  }
  default:
    UDASSERT(type() == Type::AssocArray, "Wrong type!");
    return udSlice<udKeyValuePair>();
  }
}
