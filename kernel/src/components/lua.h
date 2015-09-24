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

  epVariant GetGlobal(epVariant key) const;
  void SetGlobal(epVariant key, epVariant value);

  void Execute(epString code);
  void Print(epString str) const;

private:
  Lua(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);
  ~Lua() override;

  class LuaState *pLua = nullptr;

  StreamRef outputStream;
  StreamRef errorStream;
};

} // namespace ep

#endif // _LUA_H
