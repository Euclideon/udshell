#pragma once
#ifndef _LUA_H
#define _LUA_H

#include "component.h"
#include "stream.h"

namespace ud
{

PROTOTYPE_COMPONENT(Lua);
class LuaState;

class Lua : public Component
{
public:
  UD_COMPONENT(Lua);

  StreamRef GetOutputStream() const { return outputStream; }
  void SetOutputStream(StreamRef stream);

  StreamRef GetErrorStream() const { return errorStream; }
  void SetErrorStream(StreamRef stream);

  udVariant GetGlobal(udVariant key) const;
  void SetGlobal(udVariant value, udVariant key);

  void Execute(udString code);
  void Print(udString str) const;

private:
  Lua(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
  ~Lua() override;

  class LuaState *pLua = nullptr;

  StreamRef outputStream;
  StreamRef errorStream;
};

} // namespace ud

#endif // _LUA_H
