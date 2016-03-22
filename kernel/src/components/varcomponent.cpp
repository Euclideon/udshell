#include "components/varcomponent.h"

namespace ep {

VarComponent::VarComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : DynamicComponent(pType, pKernel, uid, initParams)
  , map(initParams)
{}

VarComponent::~VarComponent()
{}

const MethodDesc *VarComponent::GetMethodDesc(String _name, EnumerateFlags enumerateFlags) const
{
  if (!(enumerateFlags & EnumerateFlags::NoDynamic))
  {
    const Variant *pVal = map.Get(_name);
    if (pVal)
      return CacheMethodDesc(_name, _name, pVal->as<VarDelegate>());
  }
  return Super::GetMethodDesc(_name, enumerateFlags);
}

} // namespace ep
