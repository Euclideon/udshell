#pragma once
#ifndef _LUA_H
#define _LUA_H

#include "component.h"
#include "stream.h"

namespace ep
{

PROTOTYPE_COMPONENT(Lua);
class LuaState;

class Lua : public Component
{
public:
  EP_COMPONENT(Lua);

  StreamRef GetOutputStream() const { return outputStream; }
  void SetOutputStream(StreamRef stream);

  StreamRef GetErrorStream() const { return errorStream; }
  void SetErrorStream(StreamRef stream);

  Variant GetGlobal(Variant key) const;
  void SetGlobal(Variant key, Variant value);

  void Execute(String code);
  void Print(String str) const;

private:
  Lua(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  ~Lua() override;

  class LuaState *pLua = nullptr;

  StreamRef outputStream;
  StreamRef errorStream;
};

} // namespace ep

#endif // _LUA_H
