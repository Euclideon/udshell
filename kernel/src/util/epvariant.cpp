
#include "ep/epvariant.h"
#include "components/component.h"
#include "components/resources/resource.h"
#include "eplua.h"


const epVariant epInitParams::varNull;

// destructor
epVariant::~epVariant()
{
  if (ownsContent)
  {
    switch ((Type)t)
    {
      case Type::Component:
        ((ep::ComponentRef*)&p)->~epSharedPtr();
        break;
      case Type::Delegate:
        ((VarDelegate*)&p)->~VarDelegate();
        break;
      case Type::String:
        udFree(s);
        break;
      case Type::Array:
        for (size_t i = 0; i < length; ++i)
          a[i].~epVariant();
        udFree(a);
        break;
      case Type::AssocArray:
        for (size_t i = 0; i < length; ++i)
        {
          aa[i].key.~epVariant();
          aa[i].value.~epVariant();
        }
        udFree(aa);
        break;
      default:
        break;
    }
  }
}

epSharedString epVariant::stringify() const
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

ptrdiff_t epVariant::compare(const epVariant &v) const
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
      return epString(s, length).cmp(epString(v.s, v.length));
    case Type::Component:
      return c->uid.cmp(v.c->uid);
    default:
      return (char*)p - (char*)v.p;
  }
}

bool epVariant::asBool() const
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
    epString str(s, length);
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
int64_t epVariant::asInt() const
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
    return epString(s, length).parseInt();
  default:
    EPASSERT(type() == Type::Int, "Wrong type!");
    return 0;
  }
}
double epVariant::asFloat() const
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
    return epString(s, length).parseFloat();
  default:
    EPASSERT(type() == Type::Float, "Wrong type!");
    return 0.0;
  }
}
const epEnumDesc* epVariant::asEnum(size_t *pVal) const
{
  if ((Type)t == Type::Enum || (Type)t == Type::Bitfield)
  {
    *pVal = (ptrdiff_t)(length << 5) >> 5;
    return (const epEnumDesc*)p;
  }
  return nullptr;
}
ep::ComponentRef epVariant::asComponent() const
{
  switch ((Type)t)
  {
  case Type::Null:
    return ep::ComponentRef();
  case Type::Component:
    return (ep::ComponentRef&)p;
  default:
    EPASSERT(type() == Type::Component, "Wrong type!");
    return nullptr;
  }
}
epVariant::VarDelegate epVariant::asDelegate() const
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
epString epVariant::asString() const
{
  // TODO: it would be nice to be able to string-ify other types
  // ...but we don't have any output buffer
  switch ((Type)t)
  {
  case Type::Null:
    return epString();
  case Type::String:
    return epString(s, length);
  case Type::Component:
    return c->GetUid();
  default:
    EPASSERT(type() == Type::String, "Wrong type!");
    return epString();
  }
}
epSlice<epVariant> epVariant::asArray() const
{
  switch ((Type)t)
  {
  case Type::Null:
    return epSlice<epVariant>();
  case Type::Array:
    return epSlice<epVariant>(a, length);
  default:
    EPASSERT(type() == Type::Array, "Wrong type!");
    return epSlice<epVariant>();
  }
}
epSlice<epKeyValuePair> epVariant::asAssocArray() const
{
  switch ((Type)t)
  {
  case Type::Null:
    return epSlice<epKeyValuePair>();
  case Type::AssocArray:
    return epSlice<epKeyValuePair>(aa, length);
  default:
    EPASSERT(type() == Type::AssocArray, "Wrong type!");
    return epSlice<epKeyValuePair>();
  }
}
epSlice<epKeyValuePair> epVariant::asAssocArraySeries() const
{
  switch ((Type)t)
  {
  case Type::Null:
    return epSlice<epKeyValuePair>();
  case Type::AssocArray:
    return epSlice<epKeyValuePair>(aa, assocArraySeriesLen());
  default:
    EPASSERT(type() == Type::AssocArray, "Wrong type!");
    return epSlice<epKeyValuePair>();
  }
}

size_t epVariant::arrayLen() const
{
  if (is(Type::Array))
    return length;
  if (is(Type::AssocArray))
    return assocArraySeriesLen();
  return 0;
}
size_t epVariant::assocArraySeriesLen() const
{
  if (!is(Type::AssocArray))
    return 0;
  size_t i = 0;
  while (i < length && aa[i].key.is(Type::Int) && aa[i].key.i == (int64_t)i + 1)
    ++i;
  return i;
}

epVariant epVariant::operator[](size_t i) const
{
  if (is(Type::Array))
  {
    EPASSERT(i < length, "Index out of range!");
    return a[i];
  }
  if (is(Type::AssocArray))
  {
    EPASSERT(i < assocArraySeriesLen(), "Index out of range!");
    return aa[i].value;
  }
  return epVariant(nullptr);
}
epVariant epVariant::operator[](epString key) const
{
  if (is(Type::AssocArray))
  {
    size_t i = assocArraySeriesLen();
    for (; i<length; ++i)
    {
      epVariant &k = aa[i].key;
      if (!k.is(Type::String))
        continue;
      if (epString(k.s, k.length).eq(key))
        return aa[i].value;
    }
  }
  return epVariant(nullptr);
}

epVariant* epVariant::allocArray(size_t len)
{
  this->~epVariant();
  t = (size_t)Type::Array;
  length = len;
  ownsContent = true;
  a = udAllocType(epVariant, len, udAF_None);
  return a;
}

epKeyValuePair* epVariant::allocAssocArray(size_t len)
{
  this->~epVariant();
  t = (size_t)Type::AssocArray;
  length = len;
  ownsContent = true;
  aa = udAllocType(epKeyValuePair, len, udAF_None);
  return aa;
}

void epVariant::luaPush(ep::LuaState &l) const
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
    case Type::Enum:
    case Type::Bitfield:
    {
      size_t val;
      const epEnumDesc *pDesc = asEnum(&val);
      epMutableString64 s;
      pDesc->stringify(val, s);
      l.pushString(s);
      break;
    }
    case Type::Component:
      l.pushComponent(ep::ComponentRef(c));
      break;
    case Type::Delegate:
      l.pushDelegate((VarDelegate&)p);
      break;
    case Type::String:
      l.pushString(epString(s, length));
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

epVariant epVariant::luaGet(ep::LuaState &l, int idx)
{
  ep::LuaType t = l.getType(idx);
  switch (t)
  {
    case ep::LuaType::Nil:
      return epVariant();
    case ep::LuaType::Boolean:
      return epVariant(l.toBool(idx));
    case ep::LuaType::LightUserData:
      return epVariant();
    case ep::LuaType::Number:
      if (l.isInteger(idx))
        return epVariant((int64_t)l.toInt(idx));
      else
        return epVariant(l.toFloat(idx));
    case ep::LuaType::String:
      return epVariant(l.toString(idx));
    case ep::LuaType::Function:
      return l.toDelegate(idx);
    case ep::LuaType::UserData:
    {
      lua_State *L = l.state();
      if (lua_getmetatable(L, idx) == 0)
        luaL_error(L, "attempt to get 'userdata: %p' as a Component", lua_topointer(L, idx));

      lua_getfield(L, -1, "__udtype"); // must be a Component
      const char *type = lua_tostring(L, -1);
      lua_pop(L, 1);

      epVariant v;
      if (!strcmp(type, "component"))
        return epVariant(l.toComponent(idx));
      else if (!strcmp(type, "delegate"))
        return epVariant(l.toDelegate(idx));
//      else if (!strcmp(type, "event"))
//        return epVariant(l.toEvent(idx));
      return epVariant();
    }
    case ep::LuaType::Table:
    {
      lua_State *L = l.state();

      int pos = idx < 0 ? idx-1 : idx;

      // work out how many items are in the table
      // HACK: we are doing a brute-force count!
      // TODO: this should be replaced with better stuff
      size_t numElements = 0;
      l.pushNil();  // first key
      while (lua_next(L, pos) != 0)
      {
        ++numElements;
        l.pop();
      }

      // alloc for table
      epVariant v;
      epKeyValuePair *pAA = v.allocAssocArray(numElements);

      // populate the table
      l.pushNil();  // first key
      int i = 0;
      while (lua_next(L, pos) != 0)
      {
        new(&pAA[i].key) epVariant(l.get(-2));
        new(&pAA[i].value) epVariant(l.get(-1));
        l.pop();
        ++i;
      }
      return v;
    }
    default:
      // TODO: make a noise of some sort...?
      return epVariant();
  }
}

ptrdiff_t epStringifyVariant(epSlice<char> buffer, epString format, const epVariant &v, const epVarArg *pArgs)
{
  switch (v.type())
  {
    case epVariant::Type::Null:
      return epStringifyTemplate(buffer, format, nullptr, pArgs);
    case epVariant::Type::Bool:
      return epStringifyTemplate(buffer, format, v.asBool(), pArgs);
    case epVariant::Type::Int:
      return epStringifyTemplate(buffer, format, v.asInt(), pArgs);
    case epVariant::Type::Float:
      return epStringifyTemplate(buffer, format, v.asFloat(), pArgs);
    case epVariant::Type::Enum:
    case epVariant::Type::Bitfield:
      EPASSERT(false, "TODO! Please write me!");
      return 0;
    case epVariant::Type::String:
      return epStringifyTemplate(buffer, format, v.asString(), pArgs);
    case epVariant::Type::Component:
      return epStringifyTemplate(buffer, format, v.asComponent(), pArgs);
    case epVariant::Type::Delegate:
      return epStringifyTemplate(buffer, format, v.asDelegate(), pArgs);
    case epVariant::Type::Array:
      return epStringifyTemplate(buffer, format, v.asArray(), pArgs);
    case epVariant::Type::AssocArray:
      EPASSERT(false, "TODO! Please write me!");
      return 0;
  }
  return 0;
}
