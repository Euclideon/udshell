#include "ep/cpp/kernel.h"
#include "lua.h"
#include "ep/cpp/component/broadcaster.h"

#include "eplua.h"

namespace ep {

Array<const PropertyInfo> Lua::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("outputBroadcaster", getOutputBroadcaster, "Output broadcaster used by the Lua print functions", nullptr, 0),
  };
}
Array<const MethodInfo> Lua::getMethods() const
{
  return{
    EP_MAKE_METHOD(getGlobal, "Get a value from the Lua global table"),
    EP_MAKE_METHOD(setGlobal, "Set a value to the Lua global table"),
    EP_MAKE_METHOD(execute, "Execute Lua code"),
    EP_MAKE_METHOD(print, "Print to lua console"),
  };
}

Lua::Lua(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  spOutputBC = pKernel->createComponent<Broadcaster>();
  pLua = epNew(LuaState, pKernel);
}

Lua::~Lua()
{
  epDelete(pLua);
}

Variant Lua::getGlobal(Variant key) const
{
  EPASSERT(false, "TODO: look up 'key' from global table");
  return pLua->get(-1);
}
void Lua::setGlobal(Variant key, Variant value)
{
  pLua->set(key, value, LuaLocation::Global);
}

void Lua::execute(String code)
{
  pLua->exec(code);
}

void Lua::print(String str) const
{
  pLua->print(str);
}

} // namespace ep
