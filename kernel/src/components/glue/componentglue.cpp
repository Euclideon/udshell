#include "components/glue/componentglue.h"

namespace ep {

ComponentGlue::ComponentGlue(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, ComponentRef _spInstance, Variant::VarMap initParams)
  : Component(_pType, _pKernel, _uid, initParams)
{
  spInstance = _spInstance;

  // assign delegates from spInstance...
  save = spInstance->GetFunctionDelegate("Save");
  initComplete = spInstance->GetFunctionDelegate("InitComplete");
  receiveMessage = spInstance->GetFunctionDelegate("ReceiveMessage");
}

ComponentGlue::~ComponentGlue()
{
}

Variant ComponentGlue::Save() const
{
  // TODO: are we meant to call Super::Save() and merge the output?
  if (save)
    return save(nullptr);
  return Variant();
}

void ComponentGlue::ReceiveMessage(String message, String sender, const Variant &data)
{
  if (receiveMessage)
    receiveMessage({ message, sender, data });

  // TODO: are we meant to pass to Super::?
}

} // namespace ep
