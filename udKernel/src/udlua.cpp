
#include "udlua.h"
#include "kernel.h"

namespace ud
{

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


static udString s_luaInit(
  "function tprint (tbl, indent)                              \n"
  "  if not indent then indent = 0 end                        \n"
  "  for k, v in pairs(tbl) do                                \n"
  "    formatting = string.rep(\"  \", indent) .. k .. \": \" \n"
  "    if type(v) == \"table\" then                           \n"
  "      print(formatting)                                    \n"
  "      tprint(v, indent+1)                                  \n"
  "    else                                                   \n"
  "      print(formatting .. tostring(v))                     \n"
  "    end                                                    \n"
  "  end                                                      \n"
  "end                                                        \n"
);


static int SendMessage(lua_State *L)
{
  LuaState &l = (LuaState&)L;

  int numArgs = l.top();
  if (numArgs < 3)
  {
    // TODO: push result code and return 1
    return 0;
  }

  udString target = l.toString(1);
  udString sender = l.toString(2);
  udString message = l.toString(3);

  // get args (is present)
  udVariant args;
  if (numArgs >= 4)
    new(&args) udVariant(udVariant::luaGet(l, 4));

  /*udResult r = */l.kernel()->SendMessage(target, sender, message, args);

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
  udString type = l.toString(1);

  // get the init params
  udInitParams init;
  udVariant args;
  if (numArgs >= 2)
  {
    new(&args) udVariant(udVariant::luaGet(l, 2));
    if (args.type() == udVariant::Type::AssocArray)
      new(&init) udInitParams(args.asAssocArray());
  }

  ComponentRef c = nullptr;
  udResult r = l.kernel()->CreateComponent(type, init, &c);
  if (r == udR_Failure_)
    l.pushNil();
  else
    l.pushComponent(c);
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

  udString component = l.toString(1);

  ComponentRef c = l.kernel()->FindComponent(component);

  l.pushComponent(c);
  return 1;
}

#if UDPLATFORM_WINDOWS
// HAX! debug tools for windows
#pragma warning(disable: 4996)
struct Dispatch
{
  char buffer[2048];
  udString s;

  void Exec(Kernel *pKernel)
  {
    pKernel->Exec(s);
    delete this;
  }
};
static uint32_t StdinThread(void *data)
{
  Kernel *pKernel = (Kernel*)data;
  while (1)
  {
    Dispatch *pDispatch = new Dispatch;
    putc('>', stdout);
    putc(' ', stdout);
    gets(pDispatch->buffer);
    pDispatch->s = pDispatch->buffer;
    pKernel->DispatchToMainThreadAndWait(MakeDelegate(pDispatch, &Dispatch::Exec));
    putc('\n', stdout);
  }
}
#endif

LuaState::LuaState(Kernel *pKernel)
{
  L = lua_newstate(udLuaAlloc, pKernel);
  lua_atpanic(L, udLuaPanic);

  luaL_openlibs(L);

  exec(s_luaInit);

  // TODO: register things

  lua_register(L, "SendMessage", (lua_CFunction)SendMessage);
  lua_register(L, "CreateComponent", (lua_CFunction)CreateComponent);
  lua_register(L, "FindComponent", (lua_CFunction)FindComponent);

#if UDPLATFORM_WINDOWS
  // HAX! debug tool for windows...
  AllocConsole();
  freopen("CONIN$", "r", stdin);
  freopen("CONOUT$", "w", stdout);
  freopen("CONOUT$", "w", stderr);
  udCreateThread(&StdinThread, pKernel);
#endif
}


LuaState::~LuaState()
{
  lua_close(L);
}

int LuaState::udLuaPanic(lua_State *L)
{
  // TODO: Lua is panic!!
  return 0;
}

void* LuaState::udLuaAlloc(void *, void *ptr, size_t, size_t nsize)
{
  if (nsize == 0) {
    udFree(ptr);
    return nullptr;
  }
  else
    return udRealloc(ptr, nsize);
}

void LuaState::exec(udString code)
{
  int fail = luaL_loadbufferx(L, code.ptr, code.length, "command", nullptr);
  if (!fail)
    fail = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (fail)
  {
    const char *pS = lua_tolstring(L, -1, nullptr);
    udDebugPrintf("%s", pS);
    lua_pop(L, 1);
  }
}

udVariant LuaState::get(int idx)
{
  return udVariant::luaGet(*this, idx);
}

void LuaState::set(udVariant v, udVariant key, LuaLocation loc)
{
  int idx;
  switch (loc)
  {
    case LuaLocation::Global:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
      idx = -1; break;
    case LuaLocation::Top:
      idx = -1; break;
    default:
      UDASSERT(false, "Invalid location");
      return;
  }
  push(key);
  push(v);
  lua_settable(L, idx - 2);
}

void LuaState::setNil(udVariant key, LuaLocation loc)
{
  int idx;
  switch (loc)
  {
    case LuaLocation::Global:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
      idx = -1; break;
    case LuaLocation::Top:
      idx = -1; break;
    default:
      UDASSERT(false, "Invalid location");
      return;
  }
  push(key);
  pushNil();
  lua_settable(L, idx - 2);
}

void LuaState::setComponent(ComponentRef c, udVariant key, LuaLocation loc)
{
  int idx;
  switch (loc)
  {
    case LuaLocation::Global:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
      idx = -1; break;
    case LuaLocation::Top:
      idx = -1; break;
    default:
      UDASSERT(false, "Invalid location");
      return;
  }
  push(key);
  pushComponent(c);
  lua_settable(L, idx - 2);
}


// *** bind components to Lua ***
void LuaState::pushComponentMetatable(const ComponentDesc &desc)
{
  if (luaL_newmetatable(L, desc.id.ptr) == 0)
    return;

  // record the type
  pushString(desc.id);
  lua_setfield(L, -2, "__type");

  // push a destructor
  lua_pushcfunction(L, &componentCleaner);
  lua_setfield(L, -2, "__gc");

  // set the parent metatable
  if (desc.pSuperDesc)
  {
    pushComponentMetatable(*desc.pSuperDesc);
    lua_setmetatable(L, -2);
  }

  // push getters and setters
  pushString("__index");
  pushGetters(desc);
  lua_rawset(L, -3);
  pushString("__newindex");
  pushSetters(desc);
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

void LuaState::pushGetters(const ComponentDesc &desc)
{
  lua_newtable(L); // upvalue(1) == getters table
  lua_newtable(L); // upvalue(2) == members/methods table

  // add type and descriptor as members
  pushDescriptor(desc);
  lua_setfield(L, -2, "descriptor");

  // populate getters
  for (auto &p : desc.properties)
  {
    if (p.getter) // TODO: we should possibly push a function that reports an "unreadable" error
    {
      lua_pushlightuserdata(L, (void*)&p);
      lua_pushcclosure(L, &getter, 1);
      lua_setfield(L, -3, p.id.ptr);
    }
  }

  // populate methods
  for (auto &m : desc.methods)
  {
    lua_pushlightuserdata(L, (void*)&m);
    lua_pushcclosure(L, &method, 1);
    lua_setfield(L, -2, m.id.ptr);
  }

  // populate events
  for (auto &e : desc.events)
  {
    lua_pushlightuserdata(L, (void*)&e);
    lua_setfield(L, -2, e.id.ptr);
  }

  // upvalue(3) is super.__index
  if (luaL_getmetafield(L, -4, "__index") == LUA_TNIL)
    lua_pushnil(L);

  lua_pushcclosure(L, &componentIndex, 3);
}
void LuaState::pushSetters(const ComponentDesc &desc)
{
  lua_newtable(L);

  // populate setters
  for (auto &p : desc.properties)
  {
    if (p.setter) // TODO: we should possibly push a function that reports an "unwritable" error
    {
      lua_pushlightuserdata(L, (void*)&p);
      lua_pushcclosure(L, &setter, 1);
      lua_setfield(L, -2, p.id.ptr);
    }
  }

  if (luaL_getmetafield(L, -3, "__newindex") == LUA_TNIL)
    lua_pushnil(L);

  lua_pushcclosure(L, &componentNewIndex, 2);
}
void LuaState::pushDescriptor(const ComponentDesc &desc)
{
  lua_createtable(L, 0, 6);

  pushInt(desc.udVersion);
  lua_setfield(L, -2, "apiversion");
  pushInt(desc.pluginVersion);
  lua_setfield(L, -2, "pluginversion");
  pushString(desc.id);
  lua_setfield(L, -2, "id");
  pushString(desc.displayName);
  lua_setfield(L, -2, "displayname");
  pushString(desc.description);
  lua_setfield(L, -2, "description");

  // TODO: parent descriptor should also be here...

  if (desc.properties.empty())
    return;

  lua_createtable(L, 0, 0);
  size_t i = 1;
  for (auto &p : desc.properties)
  {
    lua_createtable(L, 0, 0);

    pushString(p.id);
    lua_setfield(L, -2, "id");
    pushString(p.displayName);
    lua_setfield(L, -2, "displayname");
    pushString(p.description);
    lua_setfield(L, -2, "description");

    pushInt((int)p.type.type);
    lua_setfield(L, -2, "type");
    pushInt(p.type.arrayLength);
    lua_setfield(L, -2, "arraylength");
    pushInt(p.flags);
    lua_setfield(L, -2, "flags");
    pushString(p.displayType);
    lua_setfield(L, -2, "displaytype");

    lua_seti(L, -2, i++);
  }
  lua_setfield(L, -2, "properties");
}

void LuaState::pushComponent(ComponentRef c)
{
  if (!c)
  {
    lua_pushnil(L);
    return;
  }

  new(lua_newuserdata(L, sizeof(ComponentRef))) ComponentRef(c);
  pushComponentMetatable(*c->pType);
  lua_setmetatable(L, -2);
}

static void verifyComponentType(lua_State* L, int idx)
{
  if (lua_getmetatable(L, idx) == 0)
    luaL_error(L, "attempt to get 'userdata: %p' as a Component", lua_topointer(L, idx));
/*
  // TODO: this should actually check that types are supported!
  lua_getfield(L, -1, "__type"); // must be a Component
  auto type = lua_tostring(L, -1);

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
  udFixedString64 s;
  s.concat("@", (*pComponent)->GetUid());
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
  auto field = lua_tostring(L, 2);

  // check the getter table
  lua_getfield(L, lua_upvalueindex(1), field);
  if (!lua_isnil(L, -1))
  {
    lua_pushvalue(L, 1);
    lua_call(L, 1, LUA_MULTRET);
    return lua_gettop(L) - 2;
  }
  lua_pop(L, 1);

  // check the members/method table
  lua_getfield(L, lua_upvalueindex(2), field);
  if (!lua_isnil(L, -1))
  {
    if (lua_islightuserdata(L, -1))
    {
      // events are light user data...
      EventDesc *pDesc = (EventDesc*)lua_touserdata(L, -1);
      lua_pop(L, 1);

      // push an event object
      LuaState &l = (LuaState&)L;
      l.pushEvent(l.toComponent(1), *pDesc);
    }
    return 1;
  }
  lua_pop(L, 1);

  // call super.__index
  lua_pushvalue(L, lua_upvalueindex(3));
  if (!lua_isnil(L, -1))
  {
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_call(L, 2, LUA_MULTRET);
    return lua_gettop(L) - 2;
  }

  // return nil (already on stack)
  lua_getglobal(L, "print");
  udFixedString64 errorMsg = udFixedString64::format("Error \"%s\" not found", field);
  lua_pushstring(L, errorMsg.toStringz());
  lua_call(L, 1, 0);
  return 1;
}
int LuaState::componentNewIndex(lua_State* L)
{
  auto field = lua_tostring(L, 2);

  // call setter
  lua_getfield(L, lua_upvalueindex(1), field);
  if (!lua_isnil(L, -1))
  {
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 3);
    lua_call(L, 2, LUA_MULTRET);
    return 0;
  }

  // call super.__newindex
  lua_pushvalue(L, lua_upvalueindex(2));
  if (!lua_isnil(L, -1))
  {
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_call(L, 3, LUA_MULTRET);
    return 0;
  }

  // return nil (already on stack)
  lua_getglobal(L, "print");
  udFixedString64 errorMsg = udFixedString64::format("Error \"%s\" not found", field);
  lua_pushstring(L, errorMsg.toStringz());
  lua_call(L, 1, 0);

  return 0;
}

// lua callbacks
int LuaState::getter(lua_State *L)
{
  LuaState &l = (LuaState&)L;
  const PropertyDesc *pProp = (const PropertyDesc*)l.toUserData(lua_upvalueindex(1));

  ComponentRef c = l.toComponent(1);
  udVariant v(pProp->getter.get(c.ptr()));

  ((LuaState&)L).push(v);
  return 1;
}
int LuaState::setter(lua_State *L)
{
  LuaState &l = (LuaState&)L;
  const PropertyDesc *pProp = (const PropertyDesc*)l.toUserData(lua_upvalueindex(1));

  ComponentRef c = l.toComponent(1);
  pProp->setter.set(c.ptr(), l.get(2));

  c->SignalPropertyChanged(pProp);
  return 0;
}
int LuaState::method(lua_State *L)
{
  LuaState &l = (LuaState&)L;
  const MethodDesc *pM = (const MethodDesc*)l.toUserData(lua_upvalueindex(1));

  ComponentRef c = l.toComponent(1);

  int numArgs = l.top() - 1;
  udVariant *pArgs = numArgs > 0 ? (udVariant*)alloca(sizeof(udVariant)*numArgs) : nullptr;

  for (int i = 0; i < numArgs; ++i)
    new(&pArgs[i]) udVariant(udVariant::luaGet(l, 2 + i));

  udVariant v(pM->method.call(c.ptr(), udSlice<udVariant>(pArgs, numArgs)));

  for (int i = 0; i < numArgs; ++i)
    pArgs[i].~udVariant();

  v.luaPush(l);
  return 1;
}


// *** bind delegates to Lua ***
int LuaState::delegateCleaner(lua_State* L)
{
  typedef udVariant::Delegate D;
  D *pDelegate = (D*)lua_touserdata(L, 1);
  pDelegate->~D();
  return 0;
}

void LuaState::pushDelegateMetatable()
{
  if (luaL_newmetatable(L, "udVariant::Delegate") == 0)
    return;

  // record the type
  pushString("udVariant::Delegate");
  lua_setfield(L, -2, "__type");

  // push a destructor
  lua_pushcfunction(L, &delegateCleaner);
  lua_setfield(L, -2, "__gc");

  // create a '__metatable' entry to protect the metatable against modification
  pushString("__metatable");
  lua_pushvalue(L, -2);
  lua_rawset(L, -3);
}

void LuaState::pushDelegate(const udVariant::Delegate &d)
{
  // TODO: detect if d is a lua function delegate
  //       if it is, push the lua function directly...

  new(lua_newuserdata(L, sizeof(udVariant::Delegate))) udVariant::Delegate(d);
  pushDelegateMetatable();
  lua_setmetatable(L, -2);
  lua_pushcclosure(L, &callDelegate, 1);
}

class LuaDelegate : public udDelegateMemento
{
protected:
  template<typename T>
  friend class ::udSharedPtr;

  udVariant call(udSlice<udVariant> args) const
  {
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

    udVariant v;
    if (numRet)
    {
      // get the first returned valuye (abandon any further return values)
      v = udVariant::luaGet((LuaState&)L, top + 1);

      // put the stack back how we got it
      lua_pop(L, numRet);
    }
    return v;
  }

  LuaDelegate(lua_State *L, int idx)
  {
    this->L = L;

    // we'll use the delegate pointer as a registry index
    lua_pushlightuserdata(L, this);
    lua_pushvalue(L, idx > 0 ? idx : idx-1);
    lua_settable(L, LUA_REGISTRYINDEX);

    // set the memento to the lua call shim
    FastDelegate<udVariant(udSlice<udVariant>)> shim(this, &LuaDelegate::call);
    m = shim.GetMemento();
  }

  ~LuaDelegate()
  {
    lua_pushlightuserdata(L, this);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
  }

  lua_State *L;
};

udVariant::Delegate LuaState::toDelegate(int idx)
{
  typedef udSharedPtr<LuaDelegate> LuaDelegateRef;

  // TODO: detect if the function is a cclosure
  //       if it is, return the udDelegate directly

  if (lua_isfunction(L, idx))
    return udVariant::Delegate(LuaDelegateRef::create(L, idx));
  return udVariant::Delegate();
}

int LuaState::callDelegate(lua_State *L)
{
  LuaState &l = (LuaState&)L;
  udVariant::Delegate &d = *(udVariant::Delegate*)l.toUserData(lua_upvalueindex(1));

  int numArgs = l.top();
  udVariant *pArgs = numArgs > 0 ? (udVariant*)alloca(sizeof(udVariant)*numArgs) : nullptr;

  for (int i = 0; i < numArgs; ++i)
    new(&pArgs[i]) udVariant(udVariant::luaGet(l, 1 + i));

  udVariant v(d(udSlice<udVariant>(pArgs, numArgs)));

  for (int i = 0; i < numArgs; ++i)
    pArgs[i].~udVariant();

  v.luaPush(l);
  return 1;
}


// *** bind events to Lua ***
void LuaState::pushEventMetatable()
{
  if (luaL_newmetatable(L, "udLuaEvent") == 0)
    return;

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
  LuaEvent(const ComponentRef &c, EventDesc &desc)
    : c(c), desc(desc)
  {}

  void subscribe(const udVariant::Delegate &d)
  {
    desc.ev.subscribe(c, d);
  }

private:
  ComponentRef c;
  EventDesc &desc;
};

void LuaState::pushEvent(const ComponentRef &c, EventDesc &desc)
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
  udVariant::Delegate d = l.toDelegate(2);

  pEv->subscribe(d);

  return 0;
}

} // namespace ud
