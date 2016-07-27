#include "components/glue/componentglue.h"

namespace ep {

ComponentGlue::ComponentGlue(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, ComponentRef _spInstance, Variant::VarMap initParams)
  : Component(_pType, _pKernel, _uid, initParams)
{
  spInstance = _spInstance;

  // assign delegates from spInstance...
  _save = spInstance->getFunctionDelegate("save");
  _receiveMessage = spInstance->getFunctionDelegate("receiveMessage");
}

ComponentGlue::~ComponentGlue()
{
}

Variant ComponentGlue::save() const
{
  // TODO: are we meant to call Super::save() and merge the output?
  if (_save)
    return _save(nullptr);
  return Variant();
}

void ComponentGlue::receiveMessage(String message, String sender, const Variant &data)
{
  if (_receiveMessage)
    _receiveMessage(Slice<const Variant>{ message, sender, data });

  // TODO: are we meant to pass to Super::?
}

} // namespace ep
