#include "kernel.h"
#include "lua.h"

#include "udlua.h"

namespace ud
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

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "lua",    // id
  "Lua",    // displayName
  "Lua VM", // description

  udSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // propeties
  udSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr // events
};

Lua::Lua(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  pLua = udNew(LuaState, pKernel);
}

Lua::~Lua()
{
  udDelete(pLua);
}

void Lua::SetOutputStream(StreamRef stream)
{
  outputStream = stream;

  UDASSERT(false, "TODO: work out how to override stdout in lua");
}

void Lua::SetErrorStream(StreamRef stream)
{
  errorStream = stream;

  UDASSERT(false, "TODO: work out how to override stderr in lua");
}

udVariant Lua::GetGlobal(udVariant key) const
{
  UDASSERT(false, "TODO: look up 'key' from global table");
  return pLua->get(-1);
}
void Lua::SetGlobal(udVariant value, udVariant key)
{
  pLua->set(value, key, LuaLocation::Global);
}

void Lua::Execute(udString code)
{
  pLua->exec(code);
}

void Lua::Print(udString str) const
{
  pLua->print(str);
}

} // namespace ud
