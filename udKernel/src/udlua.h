#pragma once
#if !defined(_LUA_H)

#include "components/component.h"

#include "lua.hpp"

namespace ud
{

enum class ConsoleColor
{
  Default = -1,

  Black = 0,
  DarkBlue,
  DarkGreen,
  DarkCyan,
  DarkRed,
  DarkMagenta,
  DarkYellow,
  LightGrey,
  DarkGrey,
  Blue,
  Green,
  Cyan,
  Red,
  Magenta,
  Yellow,
  White
};

void SetConsoleColor(ConsoleColor fg = ConsoleColor::Default, ConsoleColor bg = ConsoleColor::Default);


enum class LuaType : int
{
  None = LUA_TNONE,
  Nil = LUA_TNIL,
  Boolean = LUA_TBOOLEAN,
  LightUserData = LUA_TLIGHTUSERDATA,
  Number = LUA_TNUMBER,
  String = LUA_TSTRING,
  Table = LUA_TTABLE,
  Function = LUA_TFUNCTION,
  UserData = LUA_TUSERDATA,
  Thread = LUA_TTHREAD,

  Max = LUA_NUMTAGS
};

enum class LuaLocation : int
{
  Global = 0,
  Top = 1
};


class LuaState
{
public:
  LuaState(Kernel *pKernel);
  ~LuaState();

  lua_State *state();
  Kernel *kernel();

  void exec(udString code);
  void print(udString str);

  int top();

  LuaType getType(int idx = -1);
  const char *getTypeName(int idx = -1);
  const char *getTypeName(LuaType type);

  bool isInteger(int idx = -1);

  // push***
  void pushNil();
  void pushBool(bool val);
  void pushFloat(lua_Number val);
  void pushInt(lua_Integer val);
  void pushString(udString val);
  void pushLightUserData(void *val);

  void pushComponent(ComponentRef c);
  void pushDelegate(const udVariant::Delegate &d);

  void push(const udVariant &v);

  // pop***
  void pop(int count = 1);
  bool popBool();
  lua_Number popFloat();
  lua_Integer popInt();
  udString popString();
  lua_CFunction popFunction();
  void* popUserData();

  // to***
  bool toBool(int idx = -1);
  lua_Number toFloat(int idx = -1);
  lua_Integer toInt(int idx = -1);
  udString toString(int idx = -1);
  lua_CFunction toFunction(int idx = -1);
  void* toUserData(int idx = -1);

  ComponentRef toComponent(int idx = -1);
  udVariant::Delegate toDelegate(int idx = -1);

  udVariant get(int idx = -1);

  // set
  void setNil(udVariant key, LuaLocation loc = LuaLocation::Global);
  void setComponent(ComponentRef c, udVariant key, LuaLocation loc = LuaLocation::Global);

  void set(udVariant v, udVariant key, LuaLocation loc = LuaLocation::Global);

private:
  lua_State *L;

  static int udLuaPanic(lua_State *L);
  static void* udLuaAlloc(void *, void *ptr, size_t, size_t nsize);

  void pushComponentMetatable(const ComponentDesc &desc);
  void pushDescriptor(const ComponentDesc &desc);
  void pushGetters(const ComponentDesc &desc);
  void pushSetters(const ComponentDesc &desc);
  static int componentCleaner(lua_State* L);
  static int componentToString(lua_State* L);
  static int componentCompare(lua_State* L);
  static int componentIndex(lua_State* L);
  static int componentNewIndex(lua_State* L);
  static int getter(lua_State *L);
  static int setter(lua_State *L);
  static int method(lua_State *L);
  static int help(lua_State* L);

  void pushDelegateMetatable();
  static int delegateCleaner(lua_State* L);
  static int callDelegate(lua_State *L);

  void pushEventMetatable();
  void pushEventMembers();
  void pushEvent(const ComponentRef &c, EventDesc &desc);
  static int eventCleaner(lua_State* L);
  static int subscribe(lua_State* L);
};


struct LuaObject
{
private:
  int r = LUA_REFNIL;
  lua_State *L = nullptr;

protected:
  LuaObject(lua_State *L, int idx) : L(L)
  {
    lua_pushvalue(L, idx);
    r = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  void push() const
  {
    lua_rawgeti(L, LUA_REGISTRYINDEX, r);
  }

public:
  LuaObject(const LuaObject& o) : L(o.L)
  {
    o.push();
    r = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  ~LuaObject()
  {
    luaL_unref(L, LUA_REGISTRYINDEX, r);
  }

  lua_State* state() const
  {
    return L;
  }

  LuaType type() const
  {
    push();
    auto result = (LuaType)lua_type(L, -1);
    lua_pop(L, 1);
    return result;
  }

  bool isNil() const
  {
    return r == LUA_REFNIL;
  }
};

} // namespace ud

#include "udlua.inl"

#endif

