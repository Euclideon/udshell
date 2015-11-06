#include "kernel.h"
#include "lua.h"

#include "eplua.h"

namespace ep
{

static CPropertyDesc props[] =
{
  {
    {
      "outputstream", // id
      "Output Stream", // displayName
      "Output stream used by the Lua print functions", // description
    },
    &Lua::GetOutputStream, // getter
    &Lua::SetOutputStream  // setter
  },
  {
    {
      "errorstream", // id
      "Error Stream", // displayName
      "Stream where Lua errors are presented", // description
    },
    &Lua::GetOutputStream, // getter
    &Lua::SetOutputStream  // setter
  }
};
static CMethodDesc methods[] =
{
  {
    {
      "getglobal", // id
      "Get a value from the Lua global table", // description
    },
    &Lua::GetGlobal, // Method
  },
  {
    {
      "setglobal", // id
      "Set a value to the Lua global table", // description
    },
    &Lua::SetGlobal, // Method
  },
  {
    {
      "execute", // id
      "Execute Lua code", // description
    },
    &Lua::Execute, // Method
  },
  {
    {
      "print", // id
      "Print to lua console", // description
    },
    &Lua::Print, // Method
  }
};
ComponentDesc Lua::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "lua",    // id
  "Lua",    // displayName
  "Lua VM", // description

  Slice<CPropertyDesc>(props, UDARRAYSIZE(props)), // propeties
  Slice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr // events
};

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

} // namespace ep
