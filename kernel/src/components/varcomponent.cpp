#include "components/varcomponent.h"

namespace ep {

VarComponent::VarComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : DynamicComponent(pType, pKernel, uid, initParams)
  , map(initParams)
{}

VarComponent::~VarComponent()
{}

const MethodDesc *VarComponent::getMethodDesc(String _name, EnumerateFlags enumerateFlags) const
{
  if (!(enumerateFlags & EnumerateFlags::NoDynamic))
  {
    const Variant *pVal = map.get(_name);
    if (pVal)
      return CacheMethodDesc(_name, _name, pVal->as<VarDelegate>());
  }
  // TODO: this assumes that GetMethodDesc will *never* be overridden somewhere above the glue's hierarchy
  return pThis->Component::getMethodDesc(_name, enumerateFlags);
}

} // namespace ep
