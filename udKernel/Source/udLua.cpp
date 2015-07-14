
#include "udLua.h"
#include "udKernel.h"


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

  udVariant args;
  if (numArgs > 4)
    l.pop(numArgs-4); // we will only take the 4th arg as payload
  if (numArgs == 4)
  {
    struct Frame
    {
      Frame() {}
      Frame(udVariant *v, int i, int step) : v(v), i(i), step(step) {}
      udVariant *v;
      int i;
      int step;
    };
    Frame stack[64];
    int depth = 0;

    udVariant *a = &args;

read_value:
    LuaType t = l.getType(-1);
    switch (t)
    {
    case LuaType::Nil:
      *a = udVariant();
      break;
    case LuaType::Boolean:
      *a = udVariant(l.toBool(-1));
      break;
    case LuaType::LightUserData:
      *a = udVariant();
      break;
    case LuaType::Number:
      if (l.isInteger(-1))
        *a = udVariant(l.toInt(-1));
      else
        *a = udVariant(l.toFloat(-1));
      break;
    case LuaType::String:
      *a = udVariant(l.toString(-1));
      break;
    case LuaType::Function:
      *a = udVariant();
      break;
    case LuaType::UserData:
      *a = udVariant();
      break;
    case LuaType::Table:
      // work out how many items are in the table
      // HACK: we are doing a brute-force count! this should be replaced with better stuff
      size_t numElements = 0;
      l.pushNil();  // first key
      while (lua_next(l.state(), -2) != 0)
      {
        ++numElements;
        l.pop();
      }

      // populate the table
      udKeyValuePair *pAA = (udKeyValuePair*)alloca(sizeof(udKeyValuePair)*numElements);
      // TODO: check pAA i not nullptr!
      *a = udVariant(udSlice<udKeyValuePair>(pAA, numElements));
      l.pushNil();  // first key
      int i = 0;
      while (lua_next(l.state(), -2) != 0)
      {
        stack[depth++] = Frame(a, i, 0);
        a = &a->asAssocArray()[i].value;
        goto read_value;
resume_element:
        a = stack[depth].v;
        i = stack[depth].i;
        l.pop();

        stack[depth++] = Frame(a, i, 1);
        a = &a->asAssocArray()[i].key;
        goto read_value;
finished_element:
        a = stack[depth].v;
        i = stack[depth].i + 1;
      }
      break;
    }

    if (depth--)
    {
      if (stack[depth].step == 0)
        goto resume_element;
      else
        goto finished_element;
    }
  }

  udResult r = l.kernel()->SendMessage(target, sender, message, args);

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

  udString type = l.toString(1);

  // TODO: support optional a table in arg2 for init params

  udComponentRef c;
  udResult r = l.kernel()->CreateComponent(type, nullptr, &c);

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

  udComponentRef c = l.kernel()->FindComponent(component);

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

  void Exec(udKernel *pKernel)
  {
    pKernel->Exec(s);
    delete this;
  }
};
static uint32_t StdinThread(void *data)
{
  udKernel *pKernel = (udKernel*)data;
  while (1)
  {
    Dispatch *pDispatch = new Dispatch;
    gets(pDispatch->buffer);
    pDispatch->s = pDispatch->buffer;
    pKernel->DispatchToMainThread(MakeDelegate(pDispatch, &Dispatch::Exec));
  }
}
#endif

LuaState::LuaState(udKernel *pKernel)
{
  L = lua_newstate(udLuaAlloc, pKernel);
  lua_atpanic(L, udLuaPanic);

  luaL_openlibs(L);

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

void LuaState::pushComponent(udComponentRef c)
{
  // TODO: push a component, setting the metatable appropriately (which may mean creating one)
  //...
}
