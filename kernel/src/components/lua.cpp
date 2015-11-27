#include "kernel.h"
#include "lua.h"

#include "eplua.h"

namespace kernel
{

Lua::Lua(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  pLua = new LuaState(pKernel);
}

Lua::~Lua()
{
  delete pLua;
}

void Lua::SetOutputStream(StreamRef stream)
{
  outputStream = stream;

  EPASSERT(false, "TODO: work out how to override stdout in lua");
}

void Lua::SetErrorStream(StreamRef stream)
{
  errorStream = stream;

  EPASSERT(false, "TODO: work out how to override stderr in lua");
}

Variant Lua::GetGlobal(Variant key) const
{
  EPASSERT(false, "TODO: look up 'key' from global table");
  return pLua->get(-1);
}
void Lua::SetGlobal(Variant key, Variant value)
{
  pLua->set(key, value, LuaLocation::Global);
}

void Lua::Execute(String code)
{
  pLua->exec(code);
}

void Lua::Print(String str) const
{
  pLua->print(str);
}

} // namespace kernel
