
#include "udVariant.h"
#include "udComponent.h"
#include "udLua.h"

namespace udKernel
{

udRCString Variant::stringify() const
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

bool Variant::asBool() const
{
  switch ((Type)t)
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
    udString str(s, length);
    if (str.eqi("true"))
      return true;
    else if (str.eqi("false"))
      return false;
    return !str.empty();
  }
  default:
    UDASSERT(type() == Type::Bool, "Wrong type!");
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
    return udString(s, length).parseInt();
  default:
    UDASSERT(type() == Type::Int, "Wrong type!");
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
    return udString(s, length).parseFloat();
  default:
    UDASSERT(type() == Type::Float, "Wrong type!");
    return 0.0;
  }
}
ComponentRef Variant::asComponent() const
{
  switch ((Type)t)
  {
  case Type::Component:
    return ComponentRef(c);
  default:
    UDASSERT(type() == Type::Component, "Wrong type!");
    return nullptr;
  }
}
Variant::Delegate Variant::asDelegate() const
{
  switch ((Type)t)
  {
    case Type::Delegate:
      return (Delegate&)p;
    default:
      UDASSERT(type() == Type::Delegate, "Wrong type!");
      return Delegate();
  }
}
udString Variant::asString() const
{
  // TODO: it would be nice to be able to string-ify other types
  // ...but we don't have any output buffer
  switch ((Type)t)
  {
  case Type::Null:
    return udString();
  case Type::String:
    return udString(s, length);
  case Type::Component:
    return c->GetUid();
  default:
    UDASSERT(type() == Type::String, "Wrong type!");
    return udString();
  }
}
udSlice<Variant> Variant::asArray() const
{
  switch ((Type)t)
  {
  case Type::Null:
    return udSlice<Variant>();
  case Type::Array:
    return udSlice<Variant>(a, length);
  default:
    UDASSERT(type() == Type::Array, "Wrong type!");
    return udSlice<Variant>();
  }
}
udSlice<KeyValuePair> Variant::asAssocArray() const
{
  switch ((Type)t)
  {
  case Type::Null:
    return udSlice<KeyValuePair>();
  case Type::AssocArray:
    return udSlice<KeyValuePair>(aa, length);
  default:
    UDASSERT(type() == Type::AssocArray, "Wrong type!");
    return udSlice<KeyValuePair>();
  }
}
udSlice<KeyValuePair> Variant::asAssocArraySeries() const
{
  switch ((Type)t)
  {
  case Type::Null:
    return udSlice<KeyValuePair>();
  case Type::AssocArray:
    return udSlice<KeyValuePair>(aa, assocArraySeriesLen());
  default:
    UDASSERT(type() == Type::AssocArray, "Wrong type!");
    return udSlice<KeyValuePair>();
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
  while (i < length && aa[i].key.is(Type::Int) && aa[i].key.i == i + 1)
    ++i;
  return i;
}

Variant Variant::operator[](size_t i) const
{
  if (is(Type::Array))
  {
    UDASSERT(i < length, "Index out of range!");
    return a[i];
  }
  if (is(Type::AssocArray))
  {
    UDASSERT(i < assocArraySeriesLen(), "Index out of range!");
    return aa[i].value;
  }
  return Variant(nullptr);
}
Variant Variant::operator[](udString key) const
{
  if (is(Type::AssocArray))
  {
    size_t i = assocArraySeriesLen();
    for (; i<length; ++i)
    {
      Variant &k = aa[i].key;
      if (!k.is(Type::String))
        continue;
      if (udString(k.s, k.length).eq(key))
        return aa[i].value;
    }
  }
  return Variant(nullptr);
}

Variant* Variant::allocArray(size_t len)
{
  this->~Variant();
  t = (size_t)Type::Array;
  length = len;
  ownsArray = true;
  a = udAllocType(Variant, len, udAF_None);
  return a;
}

KeyValuePair* Variant::allocAssocArray(size_t len)
{
  this->~Variant();
  t = (size_t)Type::AssocArray;
  length = len;
  ownsArray = true;
  aa = udAllocType(KeyValuePair, len, udAF_None);
  return aa;
}

void Variant::luaPush(LuaState &l) const
{
  switch ((Type)t)
  {
    case Type::Null:
      l.pushNil();
      break;
    case Type::Bool:
      l.pushBool(b);
      break;
    case Type::Int:
      l.pushInt(i);
      break;
    case Type::Float:
      l.pushFloat(f);
      break;
    case Type::Component:
      l.pushComponent(ComponentRef(c));
      break;
    case Type::Delegate:
      l.pushDelegate((Delegate&)p);
      break;
    case Type::String:
      l.pushString(udString(s, length));
      break;
    case Type::Array:
    {
      lua_State *L = l.state();
      lua_createtable(L, (int)length, 0);
      for (size_t i = 0; i<length; ++i)
      {
        l.push(a[i]);
        lua_seti(L, -2, i+1);
      }
      break;
    }
    case Type::AssocArray:
    {
      lua_State *L = l.state();
      lua_createtable(L, 0, 0); // TODO: estimate narr and nrec?
      for (size_t i = 0; i<length; ++i)
      {
        l.push(aa[i].key);
        l.push(aa[i].value);
        lua_settable(L, -3);
      }
      break;
    }
  }
}

Variant Variant::luaGet(LuaState &l, int idx)
{
  LuaType t = l.getType(idx);
  switch (t)
  {
    case LuaType::Nil:
      return Variant();
    case LuaType::Boolean:
      return Variant(l.toBool(idx));
    case LuaType::LightUserData:
      return Variant();
    case LuaType::Number:
      if (l.isInteger(-1))
        return Variant(l.toInt(idx));
      else
        return Variant(l.toFloat(idx));
    case LuaType::String:
      return Variant(l.toString(idx));
    case LuaType::Function:
      return l.toDelegate(idx);
    case LuaType::UserData:
    {
      // find out if is component...
      UDASSERT(false, "TODO!");
      return Variant();
    }
    case LuaType::Table:
    {
      lua_State *L = l.state();

      int pos = idx < 0 ? idx-1 : idx;

      // work out how many items are in the table
      // HACK: we are doing a brute-force count! this should be replaced with better stuff
      size_t numElements = 0;
      l.pushNil();  // first key
      while (lua_next(L, pos) != 0)
      {
        ++numElements;
        l.pop();
      }

      // alloc for table
      Variant v;
      KeyValuePair *pAA = v.allocAssocArray(numElements);

      // populate the table
      l.pushNil();  // first key
      int i = 0;
      while (lua_next(L, pos) != 0)
      {
        new(&pAA[i].key) Variant(l.get(-2));
        new(&pAA[i].value) Variant(l.get(-1));
        l.pop();
        ++i;
      }
      return v;
    }
    default:
      // TODO: make a noise of some sort...?
      return Variant();
  }
}

} // namespace udKernel
