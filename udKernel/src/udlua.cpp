
#include "udlua.h"
#include "kernel.h"

namespace ud
{

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


#if UDPLATFORM_WINDOWS
void SetConsoleColor(ConsoleColor fg, ConsoleColor bg)
{
  if (fg == ConsoleColor::Default)
    fg = ConsoleColor::LightGrey;
  if (bg == ConsoleColor::Default)
    bg = ConsoleColor::Black;
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(hConsole, (uint8_t)fg | ((uint8_t)bg << 4));
}
#else
void SetConsoleColor(ConsoleColor fg, ConsoleColor bg)
{
  static const char *fg_codes[] = {
    "\x1b[39m", // default
    "\x1b[30m", "\x1b[34m", "\x1b[32m", "\x1b[36m", "\x1b[31m", "\x1b[35m", "\x1b[33m", "\x1b[37m", // colors
    "\x1b[30;1m", "\x1b[34;1m", "\x1b[32;1m", "\x1b[36;1m", "\x1b[31;1m", "\x1b[35;1m", "\x1b[33;1m", "\x1b[37;1m", // 'bold'
  };
  static const char *gb_codes[] = {
    "\x1b[49m", // default
    "\x1b[40m", "\x1b[44m", "\x1b[42m", "\x1b[46m", "\x1b[41m", "\x1b[45m", "\x1b[43m", "\x1b[47m", // colors
    "\x1b[40;1m", "\x1b[44;1m", "\x1b[42;1m", "\x1b[46;1m", "\x1b[41;1m", "\x1b[45;1m", "\x1b[43;1m", "\x1b[47;1m", // 'bold'
  };
  printf(fg_codes[(int)fg + 1]);
  printf(fg_codes[(int)fg + 1]);
}
#endif


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

  exec(init_lua);

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
  UDASSERT(false, "TODO: implement panic handler... throw exception?");
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

void LuaState::print(udString str)
{
  lua_getglobal(L, "print");
  lua_pushlstring(L, str.ptr, str.length);
  lua_call(L, 1, 0);
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
  switch (loc)
  {
    case LuaLocation::Global:
      lua_pop(L, 1);
      break;
    default:
      break;
  }
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
  switch (loc)
  {
    case LuaLocation::Global:
      lua_pop(L, 1);
      break;
    default:
      break;
  }
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
  switch (loc)
  {
    case LuaLocation::Global:
      lua_pop(L, 1);
      break;
    default:
      break;
  }
}


// *** bind components to Lua ***
void LuaState::pushComponentMetatable(const ComponentDesc &desc)
{
  if (luaL_newmetatable(L, desc.id.ptr) == 0)
    return;

  // record the logical type
  pushString("component");
  lua_setfield(L, -2, "__udtype");

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

  if (desc.propertyTree.Empty())
    return;

  lua_createtable(L, 0, 0);
  size_t i = 1;
  for (auto &p : desc.propertyTree)
  {
    lua_createtable(L, 0, 0);

    pushString(p.info.id);
    lua_setfield(L, -2, "id");
    pushString(p.info.displayName);
    lua_setfield(L, -2, "displayname");
    pushString(p.info.description);
    lua_setfield(L, -2, "description");

    pushInt(p.info.flags);
    lua_setfield(L, -2, "flags");
    pushString(p.info.displayType);
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
  udMutableString64 s;
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
  LuaState &l = (LuaState&)L;

  // get component
  ComponentRef spC = l.toComponent(1);

  // get field name
  size_t len;
  auto pField = lua_tolstring(L, 2, &len);
  udString field(pField, len);

  // check for getter
  const PropertyDesc *pProp = spC->GetPropertyDesc(field);
  if (pProp)
  {
    if (pProp->getter)
    {
      udVariant result(pProp->getter->get(spC.ptr()));
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
  const MethodDesc *pMethod = spC->GetMethodDesc(field);
  if (pMethod)
  {
    lua_pushlightuserdata(L, (void*)pMethod->method);
    lua_pushcclosure(L, &method, 1);
    return 1;
  }

  // check for events
  const EventDesc *pEv = spC->GetEventDesc(field);
  if (pEv)
  {
    l.pushEvent(spC, *pEv);
    return 1;
  }

  // TODO: move these to be actual methods?
  if (field.cmp(udString("descriptor")) == 0)
  {
    // TODO: return descriptor here...
    UDASSERT(false, "TODO");
//    pushDescriptor(*spC->pType);
    return 0;
  }
  else if (field.cmp(udString("help")) == 0)
  {
    lua_pushcfunction(L, &help);
    return 1;
  }

  // TODO: make better error message, this doesn't feel right
  udMutableString64 errorMsg; errorMsg.format("Error: '{0}' not found", field);
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

  // get field name
  size_t len;
  auto pField = lua_tolstring(L, 2, &len);
  udString field(pField, len);

  // check for setter
  const PropertyDesc *pProp = spC->GetPropertyDesc(field);
  if (pProp)
  {
    if (pProp->setter)
    {
      pProp->setter->set(spC.ptr(), l.get(3));
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
  udMutableString64 errorMsg; errorMsg.format("Error: '{0}' not found", field);
  l.print(errorMsg);
  return 0;
}

// lua callbacks
int LuaState::method(lua_State *L)
{
  LuaState &l = (LuaState&)L;
  const Method *pM = (const Method*)l.toUserData(lua_upvalueindex(1));

  ComponentRef c = l.toComponent(1);

  // TODO: assert that c is a component!

  int numArgs = l.top() - 1;
  udVariant *pArgs = numArgs > 0 ? (udVariant*)alloca(sizeof(udVariant)*numArgs) : nullptr;

  for (int i = 0; i < numArgs; ++i)
    new(&pArgs[i]) udVariant(udVariant::luaGet(l, 2 + i));

  udVariant v(pM->call(c.ptr(), udSlice<udVariant>(pArgs, numArgs)));

  for (int i = 0; i < numArgs; ++i)
    pArgs[i].~udVariant();

  v.luaPush(l);
  return 1;
}

int LuaState::help(lua_State* L)
{
  LuaState &l = (LuaState&)L;

  int numArgs = l.top();
  if (numArgs < 1)
    return 0;

  ComponentRef c = l.toComponent(1);
  const ComponentDesc *pDesc = c->pType;

  udMutableString256 buffer;
  if (numArgs > 1)
  {
    // help for member
    udString s = l.toString(2);

    // find member...
  }
  else
  {
    // general help
    SetConsoleColor(ConsoleColor::Cyan);
    l.print(pDesc->id);

    SetConsoleColor();
    l.print(pDesc->description);

    udMutableString64 buf;
    if (c->NumProperties() > 0)
    {
      l.print("\nProperties:");

      SetConsoleColor(ConsoleColor::Green);
      for (auto &p : c->instanceProperties)
      {
        buf.sprintf("  %-16s - %s", (const char*)p.info.id.toStringz(), (const char*)p.info.description.toStringz());
        l.print(buf);
      }
      for (auto &p : pDesc->propertyTree)
      {
        buf.sprintf("  %-16s - %s", (const char*)p.info.id.toStringz(), (const char*)p.info.description.toStringz());
        l.print(buf);
      }
    }

    if (c->NumMethods() > 0)
    {
      SetConsoleColor();
      l.print("\nMethods:");

      SetConsoleColor(ConsoleColor::Magenta);
      for (auto &m : c->instanceMethods)
      {
/*
        udMutableString64 func = udMutableString64::format("%s(", m->id.toStringz());
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
        buf.sprintf("  %-16s - %s", (const char*)m.info.id.toStringz(), (const char*)m.info.description.toStringz());
        l.print(buf);
      }
      for (auto &m : pDesc->methodTree)
      {
/*
        udMutableString64 func = udMutableString64::format("%s(", m->id.toStringz());
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
        buf.sprintf("  %-16s - %s", (const char*)m.info.id.toStringz(), (const char*)m.info.description.toStringz());
        l.print(buf);
      }
    }

    if (c->NumEvents() > 0)
    {
      SetConsoleColor();
      l.print("\nEvents:");

      SetConsoleColor(ConsoleColor::Yellow);
      for (auto &e : c->instanceEvents)
      {
        buf.sprintf("  %-16s - %s", (const char*)e.info.id.toStringz(), (const char*)e.info.description.toStringz());
        l.print(buf);
      }
      for (auto &e : pDesc->eventTree)
      {
        buf.sprintf("  %-16s - %s", (const char*)e.info.id.toStringz(), (const char*)e.info.description.toStringz());
        l.print(buf);
      }
    }

    SetConsoleColor();
  }

  return 0;
}


// *** bind delegates to Lua ***
int LuaState::delegateCleaner(lua_State* L)
{
  typedef udVariant::VarDelegate D;
  D *pDelegate = (D*)lua_touserdata(L, 1);
  pDelegate->~D();
  return 0;
}

void LuaState::pushDelegateMetatable()
{
  if (luaL_newmetatable(L, "udVariant::VarDelegate") == 0)
    return;

  // record the logical type
  pushString("delegate");
  lua_setfield(L, -2, "__udtype");

  // record the type
  pushString("udVariant::VarDelegate");
  lua_setfield(L, -2, "__type");

  // push a destructor
  lua_pushcfunction(L, &delegateCleaner);
  lua_setfield(L, -2, "__gc");

  // create a '__metatable' entry to protect the metatable against modification
  pushString("__metatable");
  lua_pushvalue(L, -2);
  lua_rawset(L, -3);
}

void LuaState::pushDelegate(const udVariant::VarDelegate &d)
{
  // TODO: detect if d is a lua function delegate
  //       if it is, push the lua function directly...

  new(lua_newuserdata(L, sizeof(udVariant::VarDelegate))) udVariant::VarDelegate(d);
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

udVariant::VarDelegate LuaState::toDelegate(int idx)
{
  typedef udSharedPtr<LuaDelegate> LuaDelegateRef;

  // TODO: detect if the function is a cclosure
  //       if it is, return the udDelegate directly

  if (lua_isfunction(L, idx))
    return udVariant::VarDelegate(LuaDelegateRef::create(L, idx));
  return udVariant::VarDelegate();
}

int LuaState::callDelegate(lua_State *L)
{
  LuaState &l = (LuaState&)L;
  udVariant::VarDelegate &d = *(udVariant::VarDelegate*)l.toUserData(lua_upvalueindex(1));

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

  void subscribe(const udVariant::VarDelegate &d)
  {
    desc.ev->subscribe(c, d);
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
  udVariant::VarDelegate d = l.toDelegate(2);

  pEv->subscribe(d);

  return 0;
}

} // namespace ud
