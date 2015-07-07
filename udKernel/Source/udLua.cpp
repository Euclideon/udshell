
#include "udLua.h"


static const char s_udatatypename[] = "userdata";
static const char *const s_luaTypes[LUA_NUMTAGS + 1] = {
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


LuaState::LuaState(udKernel *pKernel)
{
  L = lua_newstate(udLuaAlloc, pKernel);
  lua_atpanic(L, udLuaPanic);

  // register things

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

LuaType LuaState::getType(int idx)
{
  return (LuaType)lua_type(L, idx);
}

const char *LuaState::getTypeName(LuaType type)
{
  return s_luaTypes[(size_t)type + 1];
}


// to***
bool LuaState::toBool(int idx)
{
  return lua_toboolean(L, idx) != 0;
}
lua_Number LuaState::toFloat(int idx)
{
  int isnum;
  lua_Number f = lua_tonumberx(L, idx, &isnum);
  // TODO: check isnum
  return f;
}
lua_Integer LuaState::toInt(int idx)
{
  int isnum;
  lua_Integer i = lua_tointegerx(L, idx, &isnum);
  // TODO: check isnum
  return i;
}
udString LuaState::toString(int idx)
{
  size_t len;
  const char *pS = lua_tolstring(L, idx, &len);
  return udString(pS, len);
}
lua_CFunction LuaState::toFunction(int idx)
{
  return lua_tocfunction(L, idx);
}
void* LuaState::toUserData(int idx)
{
  return lua_touserdata(L, idx);
}

// pop***
bool LuaState::popBool(int idx, int num)
{
  bool r = toBool(idx);
  lua_pop(L, num);
  return r;
}
lua_Number LuaState::popFloat(int idx, int num)
{
  lua_Number r = toFloat(idx);
  lua_pop(L, num);
  return r;
}
lua_Integer LuaState::popInt(int idx, int num)
{
  lua_Integer r = toInt(idx);
  lua_pop(L, num);
  return r;
}
udString LuaState::popString(int idx, int num)
{
  udString r = toString(idx);
  lua_pop(L, num);
  return r;
}
lua_CFunction LuaState::popFunction(int idx, int num)
{
  lua_CFunction r = toFunction(idx);
  lua_pop(L, num);
  return r;
}
void* LuaState::popUserData(int idx, int num)
{
  void* r = toUserData(idx);
  lua_pop(L, num);
  return r;
}

// push***
void LuaState::pushNil()
{
  lua_pushnil(L);
}
void LuaState::pushBool(bool val)
{
  lua_pushboolean(L, val ? 1 : 0);
}
void LuaState::pushFloat(lua_Number val)
{
  lua_pushnumber(L, val);
}
void LuaState::pushInt(lua_Integer val)
{
  lua_pushinteger(L, val);
}
void LuaState::pushString(udString val)
{
  lua_pushlstring(L, val.ptr, val.length);
}
void LuaState::pushLightUserData(void *val)
{
  if (!val)
    lua_pushnil(L);
  else
    lua_pushlightuserdata(L, val);
}
