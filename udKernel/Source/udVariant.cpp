
#include "udVariant.h"
#include "udComponent.h"
#include "udLua.h"

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
    return c->GetUid();
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
    return udSlice<udKeyValuePair>(aa, assocArraySeriesLen());
  default:
    UDASSERT(type() == Type::AssocArray, "Wrong type!");
    return udSlice<udKeyValuePair>();
  }
}

size_t udVariant::assocArraySeriesLen() const
{
  if (t != Type::AssocArray)
    return 0;
  size_t i = 0;
  while (i < length && aa[i].key.t == Type::Int && aa[i].key.i == i + 1)
    ++i;
  return i;
}

size_t udVariant::arrayLen() const
{
  if (t == Type::Array)
    return length;
  if (t == Type::AssocArray)
    return assocArraySeriesLen();
  return 0;
}

udVariant udVariant::operator[](size_t i) const
{
  if (t == Type::Array)
  {
    UDASSERT(i < length, "Index out of range!");
    return a[i];
  }
  if (t == Type::AssocArray)
  {
    UDASSERT(i < assocArraySeriesLen(), "Index out of range!");
    return aa[i].value;
  }
  return udVariant(nullptr);
}

udVariant udVariant::operator[](udString key) const
{
  if (t == Type::AssocArray)
  {
    size_t i = assocArraySeriesLen();
    for (; i<length; ++i)
    {
      udVariant &k = aa[i].key;
      if (k.t != Type::String)
        continue;
      if (udString(k.s, k.length).eq(key))
        return aa[i].value;
    }
  }
  return udVariant(nullptr);
}

void udVariant::luaPush(LuaState &l) const
{
  switch (t)
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
      l.pushComponent(udComponentRef(c));
      break;
    case Type::String:
      l.pushString(udString(s, length));
      break;
    case Type::Array:
    {
      lua_State *L = l.state();
      lua_createtable(L, length, 0);
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

udVariant udVariant::luaGet(LuaState &l, int idx)
{
  LuaType t = l.getType(idx);
  switch (t)
  {
    case LuaType::Nil:
      return udVariant();
    case LuaType::Boolean:
      return udVariant(l.toBool(idx));
    case LuaType::LightUserData:
      return udVariant();
    case LuaType::Number:
      if (l.isInteger(-1))
        return udVariant(l.toInt(idx));
      else
        return udVariant(l.toFloat(idx));
    case LuaType::String:
      return udVariant(l.toString(idx));
    case LuaType::Function:
      return udVariant();
    case LuaType::UserData:
    {
      // find out if is component...
      UDASSERT(false, "TODO!");
      return udVariant();
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

      // populate the table
      udKeyValuePair *pAA = (udKeyValuePair*)udAlloc(sizeof(udKeyValuePair)*numElements);
      // TODO: check pAA i not nullptr!
      udVariant v(udSlice<udKeyValuePair>(pAA, numElements));
      v.ownsArray = true;
      l.pushNil();  // first key
      int i = 0;
      while (lua_next(L, pos) != 0)
      {
        new(&v.aa[i].key) udVariant(l.get(-2));
        new(&v.aa[i].value) udVariant(l.get(-1));
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
