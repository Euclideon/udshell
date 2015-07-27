namespace udKernel
{
inline lua_State *LuaState::state()
{
  return L;
}

inline Kernel *LuaState::kernel()
{
  void *pUD;
  lua_getallocf(L, &pUD);
  return (Kernel*)pUD;
}

inline int LuaState::top()
{
  return lua_gettop(L);
}

inline LuaType LuaState::getType(int idx)
{
  return (LuaType)lua_type(L, idx);
}
inline const char *LuaState::getTypeName(int idx)
{
  extern const char *const s_luaTypes[LUA_NUMTAGS + 1];
  return s_luaTypes[(size_t)getType(idx) + 1];
}
inline const char *LuaState::getTypeName(LuaType type)
{
  extern const char *const s_luaTypes[LUA_NUMTAGS + 1];
  return s_luaTypes[(size_t)type + 1];
}

inline bool LuaState::isInteger(int idx)
{
  return lua_isinteger(L, idx) == 1;
}

// to***
inline bool LuaState::toBool(int idx)
{
  return lua_toboolean(L, idx) != 0;
}
inline lua_Number LuaState::toFloat(int idx)
{
  int isnum;
  lua_Number f = lua_tonumberx(L, idx, &isnum);
  // TODO: check isnum
  return f;
}
inline lua_Integer LuaState::toInt(int idx)
{
  int isnum;
  lua_Integer i = lua_tointegerx(L, idx, &isnum);
  // TODO: check isnum
  return i;
}
inline udString LuaState::toString(int idx)
{
  size_t len;
  const char *pS = lua_tolstring(L, idx, &len);
  return udString(pS, len);
}
inline lua_CFunction LuaState::toFunction(int idx)
{
  return lua_tocfunction(L, idx);
}
inline void* LuaState::toUserData(int idx)
{
  return lua_touserdata(L, idx);
}

// pop***
inline void LuaState::pop(int count)
{
  lua_pop(L, count);
}
inline bool LuaState::popBool()
{
  bool r = toBool(-1);
  lua_pop(L, 1);
  return r;
}
inline lua_Number LuaState::popFloat()
{
  lua_Number r = toFloat(-1);
  lua_pop(L, 1);
  return r;
}
inline lua_Integer LuaState::popInt()
{
  lua_Integer r = toInt(-1);
  lua_pop(L, 1);
  return r;
}
inline udString LuaState::popString()
{
  udString r = toString(-1);
  lua_pop(L, 1);
  return r;
}
inline lua_CFunction LuaState::popFunction()
{
  lua_CFunction r = toFunction(-1);
  lua_pop(L, 1);
  return r;
}
inline void* LuaState::popUserData()
{
  void* r = toUserData(-1);
  lua_pop(L, 1);
  return r;
}

// push***
inline void LuaState::pushNil()
{
  lua_pushnil(L);
}
inline void LuaState::pushBool(bool val)
{
  lua_pushboolean(L, val ? 1 : 0);
}
inline void LuaState::pushFloat(lua_Number val)
{
  lua_pushnumber(L, val);
}
inline void LuaState::pushInt(lua_Integer val)
{
  lua_pushinteger(L, val);
}
inline void LuaState::pushString(udString val)
{
  lua_pushlstring(L, val.ptr, val.length);
}
inline void LuaState::pushLightUserData(void *val)
{
  if (!val)
    lua_pushnil(L);
  else
    lua_pushlightuserdata(L, val);
}
inline void LuaState::push(const Variant &v)
{
  v.luaPush(*this);
}
} // namespace udKernel
