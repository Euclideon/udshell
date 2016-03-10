
#include "eplua.h"
#include "ep/cpp/kernel.h"
#include "components/componentimpl.h"
#include "ep/cpp/component/broadcaster.h"
#include "components/lua.h"

#include "ep/cpp/variant.h"
#include "udPlatform.h"

namespace ep {

#include "init.inc"

static const char s_udatatypename[] = "userdata";
const char *const s_luaTypes[LUA_NUMTAGS + 1] = {
  "no value",
  "nil",
  "boolean",
  s_udatatypename,
  "number",
  "string",
  "table",
  "function",
  s_udatatypename,
  "thread",
};

static udMutex *s_pLuaCallMutex = nullptr;

const char* FgColor(ConsoleColor fg)
{
  static const char *fg_codes[] = {
    "\x1b[39m", // default
    "\x1b[30m", "\x1b[34m", "\x1b[32m", "\x1b[36m", "\x1b[31m", "\x1b[35m", "\x1b[33m", "\x1b[37m", // colors
    "\x1b[30;1m", "\x1b[34;1m", "\x1b[32;1m", "\x1b[36;1m", "\x1b[31;1m", "\x1b[35;1m", "\x1b[33;1m", "\x1b[37;1m", // 'bold'
  };
  return fg_codes[(int)fg + 1];
}
const char* BgColor(ConsoleColor bg)
{
  static const char *bg_codes[] = {
    "\x1b[49m", // default
    "\x1b[40m", "\x1b[44m", "\x1b[42m", "\x1b[46m", "\x1b[41m", "\x1b[45m", "\x1b[43m", "\x1b[47m", // colors
    "\x1b[40;1m", "\x1b[44;1m", "\x1b[42;1m", "\x1b[46;1m", "\x1b[41;1m", "\x1b[45;1m", "\x1b[43;1m", "\x1b[47;1m", // 'bold'
  };
  return bg_codes[(int)bg + 1];
}

static MutableString<1024> CreateStringFromArgs(lua_State *L)
{
  MutableString<1024> str;
  LuaState &l = (LuaState&)L;

  int numArgs = l.top();
  while (numArgs)
  {
    size_t len;
    const char *pS = lua_tolstring(L, -1, &len);
    str.append(String(pS, len));

    lua_pop(L, 1);
    numArgs--;
  }
  return str;  // number of results
}

static int PrintOutput(lua_State *L)
{
  LuaState &l = (LuaState&)L;

  Kernel *pKernel = l.kernel();
  auto spLua = pKernel->GetLua();

  spLua->GetOutputBroadcaster()->Write(CreateStringFromArgs(L));

  return 0;
}

static int PrintError(lua_State *L)
{
  LuaState &l = (LuaState&)L;

  Kernel *pKernel = l.kernel();
  auto spLua = pKernel->GetLua();

  pKernel->LogError(CreateStringFromArgs(L));

  return 0;
}

static int SendMessage(lua_State *L)
{
  LuaState &l = (LuaState&)L;

  int numArgs = l.top();
  if (numArgs < 3)
  {
    // TODO: push result code and return 1
    return 0;
  }

  String target = l.toString(1);
  String sender = l.toString(2);
  String message = l.toString(3);

  // get args (is present)
  Variant args;
  if (numArgs >= 4)
    new(&args) Variant(Variant::luaGet(l, 4));

  l.kernel()->SendMessage(target, sender, message, args);

  // TODO: push result and return 1?
  return 0;  // number of results
}

static int CreateComponent(lua_State *L)
{
  LuaState &l = (LuaState&)L;

  int numArgs = l.top();
  if (numArgs < 1)
  {
    // TODO: push result code and return 1
    return 0;
  }

  // get the type
  String type = l.toString(1);

  // get the init params
  Variant::VarMap init;
  Variant args;
  if (numArgs >= 2)
  {
    new(&args) Variant(Variant::luaGet(l, 2));
    if (args.is(Variant::SharedPtrType::AssocArray))
      new(&init) Variant::VarMap(args.asAssocArray());
  }

  ComponentRef c = nullptr;
  try
  {
    l.pushComponent(l.kernel()->CreateComponent(type, init));
  }
  catch (...)
  {
    l.pushNil();
  }
  return 1;
}

static int FindComponent(lua_State *L)
{
  LuaState &l = (LuaState&)L;

  int numArgs = l.top();
  if (numArgs < 1)
  {
    // TODO: push result code and return 1
    return 0;
  }

  String component = l.toString(1);

  ComponentRef c = l.kernel()->FindComponent(component);

  l.pushComponent(c);
  return 1;
}

LuaState::LuaState(Kernel *pKernel)
{
  s_pLuaCallMutex = udCreateMutex();
  L = lua_newstate(udLuaAlloc, pKernel);
  lua_atpanic(L, udLuaPanic);

  luaL_openlibs(L);

  exec(init_lua);

  pushString("_COMPONENTS");
  lua_createtable(L, 0, 0);
  lua_settable(L, LUA_REGISTRYINDEX);

  // TODO: register things

  lua_register(L, "print", (lua_CFunction)PrintOutput);
  lua_register(L, "printerror", (lua_CFunction)PrintError);
  lua_register(L, "SendMessage", (lua_CFunction)SendMessage);
  lua_register(L, "CreateComponent", (lua_CFunction)CreateComponent);
  lua_register(L, "FindComponent", (lua_CFunction)FindComponent);
}

LuaState::~LuaState()
{
  udDestroyMutex(&s_pLuaCallMutex);
  lua_close(L);
}

int LuaState::udLuaPanic(lua_State *L)
{
  EPASSERT(false, "TODO: implement panic handler... throw exception?");
  return 0;
}

namespace internal {
void *_Realloc(void *pMemory, size_t size, const char *pFile, int line);
}

void* LuaState::udLuaAlloc(void *, void *ptr, size_t, size_t nsize)
{
  if (nsize == 0) {
    epFree(ptr);
    return nullptr;
  }
  else
    return internal::_Realloc(ptr, nsize, __FILE__, __LINE__);
}

void LuaState::exec(String code)
{
  udScopeLock lock(s_pLuaCallMutex);
  int fail = luaL_loadbufferx(L, code.ptr, code.length, "command", nullptr);
  if (!fail)
    fail = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (fail)
  {
    lua_getglobal(L, "printerror");
    lua_pushvalue(L, -2);
    lua_call(L, 1, 0);
    lua_pop(L, 1);
  }
}

void LuaState::print(String str)
{
  udScopeLock lock(s_pLuaCallMutex);
  lua_getglobal(L, "print");
  lua_pushlstring(L, str.ptr, str.length);
  lua_call(L, 1, 0);
}

Variant LuaState::get(int idx)
{
  return Variant::luaGet(*this, idx);
}

void LuaState::set(Variant key, Variant v, LuaLocation loc)
{
  int idx;
  switch (loc)
  {
    case LuaLocation::Global:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
      idx = -1; break;
    case LuaLocation::Components:
      pushString("_COMPONENTS");
      lua_gettable(L, LUA_REGISTRYINDEX);
      idx = -1; break;
    case LuaLocation::Top:
      idx = -1; break;
    default:
      EPASSERT(false, "Invalid location");
      return;
  }
  push(key);
  push(v);
  lua_settable(L, idx - 2);
  switch (loc)
  {
    case LuaLocation::Global:
    case LuaLocation::Components:
      lua_pop(L, 1);
      break;
    default:
      break;
  }
}

void LuaState::setNil(Variant key, LuaLocation loc)
{
  int idx;
  switch (loc)
  {
    case LuaLocation::Global:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
      idx = -1; break;
    case LuaLocation::Components:
      pushString("_COMPONENTS");
      lua_gettable(L, LUA_REGISTRYINDEX);
      idx = -1; break;
    case LuaLocation::Top:
      idx = -1; break;
    default:
      EPASSERT(false, "Invalid location");
      return;
  }
  push(key);
  pushNil();
  lua_settable(L, idx - 2);
  switch (loc)
  {
    case LuaLocation::Global:
    case LuaLocation::Components:
      lua_pop(L, 1);
      break;
    default:
      break;
  }
}

void LuaState::setComponent(Variant key, ComponentRef c, LuaLocation loc)
{
  int idx;
  switch (loc)
  {
    case LuaLocation::Global:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
      idx = -1; break;
    case LuaLocation::Components:
      pushString("_COMPONENTS");
      lua_gettable(L, LUA_REGISTRYINDEX);
      idx = -1; break;
    case LuaLocation::Top:
      idx = -1; break;
    default:
      EPASSERT(false, "Invalid location");
      return;
  }
  push(key);
  pushComponent(c);
  lua_settable(L, idx - 2);
  switch (loc)
  {
    case LuaLocation::Global:
    case LuaLocation::Components:
      lua_pop(L, 1);
      break;
    default:
      break;
  }
}


// *** bind components to Lua ***
void LuaState::pushComponentMetatable(const ComponentDesc &desc, bool weakPtr)
{
  MutableString64 t; t.append("ep::", desc.info.id, weakPtr ? "*" : "Ref", "\0");
  if (luaL_newmetatable(L, t.ptr) == 0)
    return;

  // record the logical type
  pushString("component");
  lua_setfield(L, -2, "__udtype");

  // record the type
  pushString(desc.info.id);
  lua_setfield(L, -2, "__type");

  if (!weakPtr)
  {
    // push a destructor
    lua_pushcfunction(L, &componentCleaner);
    lua_setfield(L, -2, "__gc");
  }

  // set the parent metatable
  if (desc.pSuperDesc)
  {
    pushComponentMetatable(*desc.pSuperDesc, weakPtr);
    lua_setmetatable(L, -2);
  }

  // push getters and setters
  pushString("__index");
  lua_pushcfunction(L, &componentIndex);
  lua_rawset(L, -3);
  pushString("__newindex");
  lua_pushcfunction(L, &componentNewIndex);
  lua_rawset(L, -3);

  // compare operator
  pushString("__eq");
  lua_pushcfunction(L, &componentCompare);
  lua_rawset(L, -3);

  // to string function
  pushString("__tostring");
  lua_pushcfunction(L, &componentToString);
  lua_rawset(L, -3);

  // create a '__metatable' entry to protect the metatable against modification
  pushString("__metatable");
  lua_pushvalue(L, -2);
  lua_rawset(L, -3);
}
void LuaState::pushDescriptor(const ComponentDescInl &desc)
{
  lua_createtable(L, 0, 6);

  pushInt(desc.info.epVersion);
  lua_setfield(L, -2, "apiversion");
  pushInt(desc.info.pluginVersion);
  lua_setfield(L, -2, "pluginversion");
  pushString(desc.info.id);
  lua_setfield(L, -2, "id");
  pushString(desc.info.displayName);
  lua_setfield(L, -2, "displayname");
  pushString(desc.info.description);
  lua_setfield(L, -2, "description");

  // TODO: parent descriptor should also be here...

  if (desc.propertyTree.Empty())
    return;

  lua_createtable(L, 0, 0);
  size_t i = 1;
  for (auto p : desc.propertyTree)
  {
    lua_createtable(L, 0, 0);

    pushString(p.value.id);
    lua_setfield(L, -2, "id");
    pushString(p.value.displayName);
    lua_setfield(L, -2, "displayname");
    pushString(p.value.description);
    lua_setfield(L, -2, "description");

    pushInt(p.value.flags);
    lua_setfield(L, -2, "flags");
    pushString(p.value.uiType);
    lua_setfield(L, -2, "uitype");

    lua_seti(L, -2, i++);
  }
  lua_setfield(L, -2, "properties");
}

void LuaState::pushComponent(const ComponentRef &c)
{
  if (!c)
  {
    lua_pushnil(L);
    return;
  }

  new(lua_newuserdata(L, sizeof(ComponentRef))) ComponentRef(c);
  pushComponentMetatable(*c->GetDescriptor(), false);
  lua_setmetatable(L, -2);
}

void LuaState::pushComponent(Component *pC)
{
  if (!pC)
  {
    lua_pushnil(L);
    return;
  }

  new(lua_newuserdata(L, sizeof(Component*))) Component*(pC);
  pushComponentMetatable(*pC->GetDescriptor(), true);
  lua_setmetatable(L, -2);
}

static void verifyComponentType(lua_State* L, int idx)
{
  if (lua_getmetatable(L, idx) == 0)
    luaL_error(L, "attempt to get 'userdata: %p' as a Component", lua_topointer(L, idx));
/*
  // TODO: this should actually check that types are supported!
  lua_getfield(L, -1, "__type"); // must be a Component
  const char *type = lua_tostring(L, -1);

  // find type in the descriptor hierarchy

  luaL_error(L, `attempt to get instance %s as type "%s"`, cname, T.stringof.ptr);
*/
  lua_pop(L, 1);
}
ComponentRef LuaState::toComponent(int idx)
{
  verifyComponentType(state(), idx);
  return *(ComponentRef*)lua_touserdata(L, idx);
}

int LuaState::componentCleaner(lua_State* L)
{
  ComponentRef *pComponent = (ComponentRef*)lua_touserdata(L, 1);
  pComponent->~ComponentRef();
  return 0;
}
int LuaState::componentToString(lua_State* L)
{
  ComponentRef *pComponent = (ComponentRef*)lua_touserdata(L, 1);
  MutableString64 s(Concat, "@", (*pComponent)->GetUid());
  lua_pushlstring(L, s.ptr, s.length);
  return 1;
}
int LuaState::componentCompare(lua_State* L)
{
  ComponentRef *pComponent1 = (ComponentRef*)lua_touserdata(L, 1);
  ComponentRef *pComponent2 = (ComponentRef*)lua_touserdata(L, 2);
  lua_pushboolean(L, (*pComponent1).ptr() == (*pComponent2).ptr());
  return 1;
}

int LuaState::componentIndex(lua_State* L)
{
  LuaState &l = (LuaState&)L;

  // get component
  ComponentRef spC = l.toComponent(1);
  ComponentImpl *pCImpl = (ComponentImpl*)spC->pImpl.ptr();

  // get field name
  size_t len;
  auto pField = lua_tolstring(L, 2, &len);
  String field(pField, len);

  // check for getter
  const PropertyDesc *pProp = pCImpl->GetPropertyDesc(field);
  if (pProp)
  {
    if (pProp->getter)
    {
      Variant result(pProp->getter.get(spC.ptr()));
      l.push(result);
      return 1;
    }
    else
    {
      // TODO: complain that property is not readable!
      lua_pushnil(L);
      return 1;
    }
  }

  // check for method
  const MethodDesc *pMethod = pCImpl->GetMethodDesc(field);
  if (pMethod)
  {
    lua_pushlightuserdata(L, (void*)&pMethod->method);
    lua_pushcclosure(L, &method, 1);
    return 1;
  }

  // check for events
  const EventDesc *pEv = pCImpl->GetEventDesc(field);
  if (pEv)
  {
    l.pushEvent(spC, *pEv);
    return 1;
  }

  // TODO: move these to be actual methods?
  if (field.cmp(String("descriptor")) == 0)
  {
    // TODO: return descriptor here...
    EPASSERT(false, "TODO");
//    pushDescriptor(*spC->pType);
    return 0;
  }
  else if (field.cmp(String("help")) == 0)
  {
    lua_pushcfunction(L, &help);
    return 1;
  }

  // TODO: make better error message, this doesn't feel right
  MutableString64 errorMsg(Format, "Error: '{0}' not found", field);
  l.print(errorMsg);

  // return nil
  lua_pushnil(L);
  return 1;
}
int LuaState::componentNewIndex(lua_State* L)
{
  LuaState &l = (LuaState&)L;

  // get component
  ComponentRef spC = l.toComponent(1);
  ComponentImpl *pCImpl = (ComponentImpl*)spC->pImpl.ptr();

  // get field name
  size_t len;
  auto pField = lua_tolstring(L, 2, &len);
  String field(pField, len);

  // check for setter
  const PropertyDesc *pProp = pCImpl->GetPropertyDesc(field);
  if (pProp)
  {
    if (pProp->setter)
    {
      pProp->setter.set(spC.ptr(), l.get(3));
      // TODO: put signal back
//      spC->SignalPropertyChanged(pProp);
      return 0;
    }
    else
    {
      // TODO: complain that property is not writable!
      return 0;
    }
  }

  // return nil (already on stack)
  MutableString64 errorMsg(Format, "Error: '{0}' not found", field);
  l.print(errorMsg);
  return 0;
}

// lua callbacks
int LuaState::method(lua_State *L)
{
  LuaState &l = (LuaState&)L;
  const MethodShim *pM = (const MethodShim*)l.toUserData(lua_upvalueindex(1));

  ComponentRef c = l.toComponent(1);

  // TODO: assert that c is a component!

  int numArgs = l.top() - 1;
  Variant *pArgs = nullptr;
  if(numArgs > 0)
    pArgs = (Variant*)alloca(sizeof(Variant)*numArgs);

  for (int i = 0; i < numArgs; ++i)
    new(&pArgs[i]) Variant(Variant::luaGet(l, 2 + i));

  Variant v(pM->call(c.ptr(), Slice<Variant>(pArgs, numArgs)));

  for (int i = 0; i < numArgs; ++i)
    pArgs[i].~Variant();

  if (v.type() != Variant::Type::Void)
  {
    v.luaPush(l);
    return 1;
  }
  return 0;
}

int LuaState::help(lua_State* L)
{
  LuaState &l = (LuaState&)L;

  int numArgs = l.top();
  if (numArgs < 1)
    return 0;

  ComponentRef c = l.toComponent(1);
  ComponentImpl *pCImpl = (ComponentImpl*)c->pImpl.ptr();
  const ComponentDescInl *pDesc = (const ComponentDescInl*)pCImpl->GetDescriptor();

  MutableString256 buffer;
  if (numArgs > 1)
  {
#if 0
    // help for member
    String s = l.toString(2);

    // find member...
#endif // 0
  }
  else
  {
    // general help
    //FgColor(ConsoleColor::Cyan); // TODO: emit ansi color
    l.print(pDesc->info.id);

    //FgColor(); // TODO: emit ansi color
    l.print(pDesc->info.description);

    MutableString64 buf;
    if (pCImpl->NumProperties() > 0)
    {
      l.print("\nProperties:");

      //FgColor(ConsoleColor::Green); // TODO: emit ansi color
      for (auto p : pCImpl->instanceProperties)
      {
        buf.sprintf("  %-16s - %s", (const char*)p.value.id.toStringz(), (const char*)p.value.description.toStringz());
        l.print(buf);
      }
      for (auto p : pDesc->propertyTree)
      {
        buf.sprintf("  %-16s - %s", (const char*)p.value.id.toStringz(), (const char*)p.value.description.toStringz());
        l.print(buf);
      }
    }

    if (pCImpl->NumMethods() > 0)
    {
      //FgColor(); // TODO: emit ansi color
      l.print("\nMethods:");

      //FgColor(ConsoleColor::Magenta); // TODO: emit ansi color
      for (auto m : pCImpl->instanceMethods)
      {
/*
        MutableString64 func = MutableString64::format("%s(", m->id.toStringz());
        for (size_t i = 0; i < m->args.length; ++i)
        {
          func.concat(m->args[i].name);
          if (i<m->args.length-1)
            func.concat(", ");
        }
        if (m->result.type != PropertyType::Void)
          func.concat(") -> ", m->result.name);
        else
          func.concat(")");
*/
        buf.sprintf("  %-16s - %s", (const char*)m.value.id.toStringz(), (const char*)m.value.description.toStringz());
        l.print(buf);
      }
      for (auto m : pDesc->methodTree)
      {
/*
        MutableString64 func = MutableString64::format("%s(", m->id.toStringz());
        for (size_t i = 0; i < m->args.length; ++i)
        {
          func.concat(m->args[i].name);
          if (i<m->args.length-1)
            func.concat(", ");
        }
        if (m->result.type != PropertyType::Void)
          func.concat(") -> ", m->result.name);
        else
          func.concat(")");
*/
        buf.sprintf("  %-16s - %s", (const char*)m.value.id.toStringz(), (const char*)m.value.description.toStringz());
        l.print(buf);
      }
    }

    if (pCImpl->NumEvents() > 0)
    {
      //FgColor(); // TODO: emit ansi color
      l.print("\nEvents:");

      //FgColor(ConsoleColor::Yellow); // TODO: emit ansi color
      for (auto e : pCImpl->instanceEvents)
      {
        buf.sprintf("  %-16s - %s", (const char*)e.value.id.toStringz(), (const char*)e.value.description.toStringz());
        l.print(buf);
      }
      for (auto e : pDesc->eventTree)
      {
        buf.sprintf("  %-16s - %s", (const char*)e.value.id.toStringz(), (const char*)e.value.description.toStringz());
        l.print(buf);
      }
    }

    //FgColor(); // TODO: emit ansi color
  }

  return 0;
}


// *** bind delegates to Lua ***
int LuaState::delegateCleaner(lua_State* L)
{
  typedef VarDelegate D;
  D *pDelegate = (D*)lua_touserdata(L, 1);
  pDelegate->~D();
  return 0;
}

void LuaState::pushDelegateMetatable()
{
  if (luaL_newmetatable(L, "VarDelegate") == 0)
    return;

  // record the logical type
  pushString("delegate");
  lua_setfield(L, -2, "__udtype");

  // record the type
  pushString("VarDelegate");
  lua_setfield(L, -2, "__type");

  // push a destructor
  lua_pushcfunction(L, &delegateCleaner);
  lua_setfield(L, -2, "__gc");

  // create a '__metatable' entry to protect the metatable against modification
  pushString("__metatable");
  lua_pushvalue(L, -2);
  lua_rawset(L, -3);
}

void LuaState::pushDelegate(const VarDelegate &d)
{
  // TODO: detect if d is a lua function delegate
  //       if it is, push the lua function directly...

  new(lua_newuserdata(L, sizeof(VarDelegate))) VarDelegate(d);
  pushDelegateMetatable();
  lua_setmetatable(L, -2);
  lua_pushcclosure(L, &callDelegate, 1);
}

class LuaDelegate : public DelegateMemento
{
protected:
  template<typename T>
  friend struct SharedPtr;

  Variant call(Slice<const Variant> args) const
  {
    udScopeLock lock(s_pLuaCallMutex);

    // there may already be elements on the stack
    int top = lua_gettop(L);

    // get the function to call
    lua_pushlightuserdata(L, (void*)this);
    lua_gettable(L, LUA_REGISTRYINDEX);

    // push each arg
    for (auto &a: args)
      a.luaPush((LuaState&)L);

    // call the function
    lua_call(L, (int)args.length, LUA_MULTRET);

    // get number of return values
    int numRet = lua_gettop(L) - top;
    if (numRet)
    {
      // get the first returned valuye (abandon any further return values)
      Variant v = Variant::luaGet((LuaState&)L, top + 1);

      // put the stack back how we got it
      lua_pop(L, numRet);

      return std::move(v);
    }
    return Variant();
  }

  LuaDelegate(lua_State *L, int idx)
  {
    udScopeLock lock(s_pLuaCallMutex);

    this->L = L;

    // we'll use the delegate pointer as a registry index
    lua_pushlightuserdata(L, this);
    lua_pushvalue(L, idx > 0 ? idx : idx-1);
    lua_settable(L, LUA_REGISTRYINDEX);

    // set the memento to the lua call shim
    FastDelegate<Variant(Slice<const Variant>)> shim(this, &LuaDelegate::call);
    m = shim.GetMemento();
  }

  ~LuaDelegate()
  {
    udScopeLock lock(s_pLuaCallMutex);

    lua_pushlightuserdata(L, this);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
  }

  lua_State *L;
};

VarDelegate LuaState::toDelegate(int idx)
{
  typedef SharedPtr<LuaDelegate> LuaDelegateRef;

  // TODO: detect if the function is a cclosure
  //       if it is, return the Delegate directly

  if (lua_isfunction(L, idx))
    return VarDelegate(LuaDelegateRef::create(L, idx));
  return VarDelegate();
}

int LuaState::callDelegate(lua_State *L)
{
  LuaState &l = (LuaState&)L;
  VarDelegate &d = *(VarDelegate*)l.toUserData(lua_upvalueindex(1));

  int numArgs = l.top();
  Variant *pArgs = numArgs > 0 ? (Variant*)alloca(sizeof(Variant)*numArgs) : nullptr;

  for (int i = 0; i < numArgs; ++i)
    new(&pArgs[i]) Variant(Variant::luaGet(l, 1 + i));

  Variant v(d(Slice<Variant>(pArgs, numArgs)));

  for (int i = 0; i < numArgs; ++i)
    pArgs[i].~Variant();

  v.luaPush(l);
  return 1;
}


// *** bind events to Lua ***
void LuaState::pushEventMetatable()
{
  if (luaL_newmetatable(L, "udLuaEvent") == 0)
    return;

  // record the logical type
  pushString("event");
  lua_setfield(L, -2, "__udtype");

  // record the type
  pushString("udLuaEvent");
  lua_setfield(L, -2, "__type");

  // push a destructor
  lua_pushcfunction(L, &eventCleaner);
  lua_setfield(L, -2, "__gc");

  // populate __index with members
  pushString("__index");
  pushEventMembers();
  lua_rawset(L, -3);

  // create a '__metatable' entry to protect the metatable against modification
  pushString("__metatable");
  lua_pushvalue(L, -2);
  lua_rawset(L, -3);
}

void LuaState::pushEventMembers()
{
  lua_createtable(L, 0, 2);

  lua_pushcfunction(L, &subscribe);
  lua_setfield(L, -2, "subscribe");
}

class LuaEvent
{
public:
  LuaEvent(const ComponentRef &c, const EventDesc &desc)
    : c(c), desc(desc)
  {}

  void subscribe(const VarDelegate &d)
  {
    desc.ev.subscribe(c.ptr(), d);
  }

private:
  ComponentRef c;
  const EventDesc &desc;

  LuaEvent& operator=(const LuaEvent &) = delete;
};

void LuaState::pushEvent(const ComponentRef &c, const EventDesc &desc)
{
  new(lua_newuserdata(L, sizeof(LuaEvent))) LuaEvent(c, desc);
  pushEventMetatable();
  lua_setmetatable(L, -2);
}

int LuaState::eventCleaner(lua_State* L)
{
  LuaEvent *pEv = (LuaEvent*)lua_touserdata(L, 1);
  pEv->~LuaEvent();
  return 0;
}

int LuaState::subscribe(lua_State* L)
{
  LuaState &l = (LuaState&)L;

  LuaEvent *pEv = (LuaEvent*)lua_touserdata(L, 1);
  VarDelegate d = l.toDelegate(2);

  pEv->subscribe(d);

  return 0;
}

} // namespace ep


// HAX: define Variant::Lua functions here to reduce include spam
void Variant::luaPush(LuaState &l) const
{
  switch ((Type)t)
  {
    case Type::Void:
      // void pushes nothing!
      // TODO: consider; should this really be supported?
      //       in a sence, we're treating this void variant like a valid value...
      break;
    case Type::Null:
      l.pushNil();
      break;
    case Type::Bool:
      l.pushBool(b ? true : false);
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
      const EnumDesc *pDesc = asEnum(&val);
      MutableString64 str;
      pDesc->stringify(val, str);
      l.pushString(str);
      break;
    }
    case Type::SharedPtr:
    {
      switch ((SharedPtrType)length)
      {
        case SharedPtrType::Unknown:
          EPASSERT(false, "TODO!");
          break;
        case SharedPtrType::Component:
          if (ownsContent)
            l.pushComponent((ComponentRef&)c);
          else
            l.pushComponent(c);
          break;
        case SharedPtrType::Delegate:
          l.pushDelegate((VarDelegate&)d);
          break;
        case SharedPtrType::Subscription:
          EPASSERT(false, "TODO!");
          break;
        case SharedPtrType::AssocArray:
        {
          lua_State *L = l.state();
          lua_createtable(L, 0, 0); // TODO: estimate narr and nrec?
          VarMap &m = (VarMap&)p;
          for (auto kvp : m)
          {
            l.push(kvp.key);
            l.push(kvp.value);
            lua_settable(L, -3);
          }
          break;
        }
      }
      break;
    }
    case Type::String:
      l.pushString(String(s, length));
      break;
    case Type::SmallString:
    {
      uint8_t *pBuffer = (uint8_t*)this;
      l.pushString(String((char*)pBuffer + 1, pBuffer[0] >> 4));
      break;
    }
    case Type::Array:
    {
      lua_State *L = l.state();
      lua_createtable(L, (int)length, 0);
      for (size_t j = 0; j<length; ++j)
      {
        l.push(a[j]);
        lua_seti(L, -2, j+1);
      }
      break;
    }
    default:
      EPUNREACHABLE;
  }
}

Variant Variant::luaGet(LuaState &l, int idx)
{
  switch (l.getType(idx))
  {
    case LuaType::Nil:
      return Variant(nullptr);
    case LuaType::Boolean:
      return Variant(l.toBool(idx));
    case LuaType::LightUserData:
      return Variant((int64_t)(size_t)l.toUserData(idx));
    case LuaType::Number:
      if (l.isInteger(idx))
        return Variant((int64_t)l.toInt(idx));
      else
        return Variant(l.toFloat(idx));
    case LuaType::String:
      return Variant(l.toString(idx));
    case LuaType::Function:
      return l.toDelegate(idx);
    case LuaType::UserData:
    {
      lua_State *L = l.state();
      if (lua_getmetatable(L, idx) == 0)
        luaL_error(L, "attempt to get 'userdata: %p' as a Component", lua_topointer(L, idx));

      lua_getfield(L, -1, "__udtype"); // must be a Component
      const char *type = lua_tostring(L, -1);
      lua_pop(L, 1);

      Variant v;
      if (!strcmp(type, "component"))
        return Variant(l.toComponent(idx));
      else if (!strcmp(type, "delegate"))
        return Variant(l.toDelegate(idx));
//      else if (!strcmp(type, "event"))
//        return Variant(l.toEvent(idx));
      return Variant(nullptr);
    }
    case LuaType::Table:
    {
      lua_State *L = l.state();

      lua_len(L, idx);
      lua_Integer len = l.popInt();

      int pos = idx < 0 ? idx-1 : idx;

      // alloc for table
      VarMap m;

      // populate the table
      l.pushNil();  // first key
      while (lua_next(L, pos) != 0)
      {
        m.Insert(l.get(-2), l.get(-1));
        l.pop();
      }

      // construct a variant
      Variant v(std::move(m));
      v.length = len;
      return std::move(v);
    }
    default:
      // TODO: make a noise of some sort...?
      return Variant();
  }
}
