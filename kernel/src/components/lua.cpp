#include "ep/cpp/kernel.h"
#include "lua.h"
#include "ep/cpp/component/broadcaster.h"

#include "eplua.h"

namespace ep {

Array<const PropertyInfo> Lua::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO(OutputBroadcaster, "Output broadcaster used by the Lua print functions", nullptr, 0),
  };
}
Array<const MethodInfo> Lua::GetMethods() const
{
  return{
    EP_MAKE_METHOD(GetGlobal, "Get a value from the Lua global table"),
    EP_MAKE_METHOD(SetGlobal, "Set a value to the Lua global table"),
    EP_MAKE_METHOD(Execute, "Execute Lua code"),
    EP_MAKE_METHOD(Print, "Print to lua console"),
  };
}

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
