
#include "util/udvariant.h"
#include "components/component.h"
#include "components/resources/resource.h"
#include "udlua.h"


const udVariant udInitParams::varNull;

// destructor
udVariant::~udVariant()
{
  if (ownsContent)
  {
    switch ((Type)t)
    {
      case Type::Component:
        ((ud::ComponentRef*)&p)->~udSharedPtr();
        break;
      case Type::Delegate:
        ((Delegate*)&p)->~Delegate();
        break;
      case Type::String:
        udFree(s);
        break;
      case Type::Array:
        for (size_t i = 0; i < length; ++i)
          a[i].~udVariant();
        udFree(a);
        break;
      case Type::AssocArray:
        for (size_t i = 0; i < length; ++i)
        {
          aa[i].key.~udVariant();
          aa[i].value.~udVariant();
        }
        udFree(aa);
        break;
      default:
        break;
    }
  }
}

udRCString udVariant::stringify() const
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

ptrdiff_t udVariant::compare(const udVariant &v) const
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
      return i - v.i;
    case Type::Float:
      return f < v.f ? -1 : (f > v.f ? 1 : 0);
    case Type::String:
      return udString(s, length).cmp(udString(v.s, v.length));
    case Type::Component:
      return c->uid.cmp(v.c->uid);
    default:
      return (char*)p - (char*)v.p;
  }
}

bool udVariant::asBool() const
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
int64_t udVariant::asInt() const
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
double udVariant::asFloat() const
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
ud::ComponentRef udVariant::asComponent() const
{
  switch ((Type)t)
  {
  case Type::Null:
    return ud::ComponentRef();
  case Type::Component:
    return (ud::ComponentRef&)p;
  default:
    UDASSERT(type() == Type::Component, "Wrong type!");
    return nullptr;
  }
}
udVariant::Delegate udVariant::asDelegate() const
{
  switch ((Type)t)
  {
    case Type::Null:
      return Delegate();
    case Type::Delegate:
      return (Delegate&)p;
    default:
      UDASSERT(type() == Type::Delegate, "Wrong type!");
      return Delegate();
  }
}
udString udVariant::asString() const
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
udSlice<udVariant> udVariant::asArray() const
{
  switch ((Type)t)
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
  switch ((Type)t)
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
  switch ((Type)t)
  {
  case Type::Null:
    return udSlice<udKeyValuePair>();
  case Type::AssocArray:
    return udSlice<udKeyValuePair>(aa, assocArraySeriesLen());
  default:
    UDASSERT(type() == Type::AssocArray, "Wrong type!");
    return udSlice<udKeyValuePair>();
  }
}

size_t udVariant::arrayLen() const
{
  if (is(Type::Array))
    return length;
  if (is(Type::AssocArray))
    return assocArraySeriesLen();
  return 0;
}
size_t udVariant::assocArraySeriesLen() const
{
  if (!is(Type::AssocArray))
    return 0;
  size_t i = 0;
  while (i < length && aa[i].key.is(Type::Int) && aa[i].key.i == (int64_t)i + 1)
    ++i;
  return i;
}

udVariant udVariant::operator[](size_t i) const
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
  return udVariant(nullptr);
}
udVariant udVariant::operator[](udString key) const
{
  if (is(Type::AssocArray))
  {
    size_t i = assocArraySeriesLen();
    for (; i<length; ++i)
    {
      udVariant &k = aa[i].key;
      if (!k.is(Type::String))
        continue;
      if (udString(k.s, k.length).eq(key))
        return aa[i].value;
    }
  }
  return udVariant(nullptr);
}

udVariant* udVariant::allocArray(size_t len)
{
  this->~udVariant();
  t = (size_t)Type::Array;
  length = len;
  ownsContent = true;
  a = udAllocType(udVariant, len, udAF_None);
  return a;
}

udKeyValuePair* udVariant::allocAssocArray(size_t len)
{
  this->~udVariant();
  t = (size_t)Type::AssocArray;
  length = len;
  ownsContent = true;
  aa = udAllocType(udKeyValuePair, len, udAF_None);
  return aa;
}

void udVariant::luaPush(ud::LuaState &l) const
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
      l.pushComponent(ud::ComponentRef(c));
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

udVariant udVariant::luaGet(ud::LuaState &l, int idx)
{
  ud::LuaType t = l.getType(idx);
  switch (t)
  {
    case ud::LuaType::Nil:
      return udVariant();
    case ud::LuaType::Boolean:
      return udVariant(l.toBool(idx));
    case ud::LuaType::LightUserData:
      return udVariant();
    case ud::LuaType::Number:
      if (l.isInteger(-1))
        return udVariant(l.toInt(idx));
      else
        return udVariant(l.toFloat(idx));
    case ud::LuaType::String:
      return udVariant(l.toString(idx));
    case ud::LuaType::Function:
      return l.toDelegate(idx);
    case ud::LuaType::UserData:
    {
      // find out if is component...
      UDASSERT(false, "TODO!");
      return udVariant();
    }
    case ud::LuaType::Table:
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
      udVariant v;
      udKeyValuePair *pAA = v.allocAssocArray(numElements);

      // populate the table
      l.pushNil();  // first key
      int i = 0;
      while (lua_next(L, pos) != 0)
      {
        new(&pAA[i].key) udVariant(l.get(-2));
        new(&pAA[i].value) udVariant(l.get(-1));
        l.pop();
        ++i;
      }
      return v;
    }
    default:
      // TODO: make a noise of some sort...?
      return udVariant();
  }
}
