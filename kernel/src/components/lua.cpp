#include "ep/cpp/kernel.h"
#include "lua.h"
#include "ep/cpp/component/broadcaster.h"

#include "eplua.h"

namespace ep
{

Lua::Lua(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  spOutputBC = pKernel->CreateComponent<Broadcaster>();
  pLua = epNew LuaState(pKernel);
}

Lua::~Lua()
{
  epDelete pLua;
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

} // namespace ep
