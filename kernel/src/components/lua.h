#pragma once
#ifndef _LUA_H
#define _LUA_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/stream.h"

namespace ep
{

SHARED_CLASS(Lua);
SHARED_CLASS(Broadcaster);
class LuaState;

class Lua : public Component
{
  EP_DECLARE_COMPONENT(ep, Lua, Component, EPKERNEL_PLUGINVERSION, "Lua VM", 0)
public:

  BroadcasterRef GetOutputBroadcaster() const { return spOutputBC; }

  Variant GetGlobal(Variant key) const;
  void SetGlobal(Variant key, Variant value);

  void Execute(String code);
  void Print(String str) const;

private:
  Lua(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~Lua() override;

  class LuaState *pLua = nullptr;

  BroadcasterRef spOutputBC = nullptr;

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

} // namespace ep

#endif // _LUA_H
