#pragma once
#if !defined(_EP_VARCOMPONENT_H)
#define _EP_VARCOMPONENT_H

#include "components/dynamiccomponent.h"

namespace ep {

class VarComponent : public DynamicComponent
{
  EP_DECLARE_COMPONENT(ep, VarComponent, DynamicComponent, EPKERNEL_PLUGINVERSION, "Component that wraps a VarMap", 0)
public:

  const PropertyDesc *getPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pThis->Component::getPropertyDesc(_name, enumerateFlags); }
  const MethodDesc *getMethodDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final;
  const EventDesc *getEventDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pThis->Component::getEventDesc(_name, enumerateFlags); }
  const StaticFuncDesc *getStaticFuncDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pThis->Component::getStaticFuncDesc(_name, enumerateFlags); }

protected:
  VarComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~VarComponent();

  Variant::VarMap map;
};

}

#endif // _EP_VARCOMPONENT_H
