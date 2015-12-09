#pragma once
#ifndef _LUA_H
#define _LUA_H

#include "ep/cpp/component.h"
#include "stream.h"

namespace ep
{

SHARED_CLASS(Lua);
class LuaState;

class Lua : public Component
{
  EP_DECLARE_COMPONENT(Lua, Component, EPKERNEL_PLUGINVERSION, "Lua VM")
public:

  StreamRef GetOutputStream() const { return outputStream; }
  void SetOutputStream(StreamRef stream);

  StreamRef GetErrorStream() const { return errorStream; }
  void SetErrorStream(StreamRef stream);

  Variant GetGlobal(Variant key) const;
  void SetGlobal(Variant key, Variant value);

  void Execute(String code);
  void Print(String str) const;

private:
  Lua(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~Lua() override;

  class LuaState *pLua = nullptr;

  StreamRef outputStream;
  StreamRef errorStream;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(OutputStream, "Output stream used by the Lua print functions", nullptr, 0),
      EP_MAKE_PROPERTY(ErrorStream, "Stream where Lua errors are presented", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(GetGlobal, "Get a value from the Lua global table"),
      EP_MAKE_METHOD(SetGlobal, "Set a value to the Lua global table"),
      EP_MAKE_METHOD(Execute, "Execute Lua code"),
      EP_MAKE_METHOD(Print, "Print to lua console"),
    };
  }
};

} // namespace ep

#endif // _LUA_H
