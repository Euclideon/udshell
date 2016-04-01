#pragma once
#if !defined(_EP_VARCOMPONENT_H)
#define _EP_VARCOMPONENT_H

#include "components/dynamiccomponent.h"

namespace ep {

class VarComponent : public DynamicComponent
{
  EP_DECLARE_COMPONENT(ep, VarComponent, DynamicComponent, EPKERNEL_PLUGINVERSION, "Component that wraps a VarMap", 0)
public:

  const PropertyDesc *GetPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pThis->Component::GetPropertyDesc(_name, enumerateFlags); }
  const MethodDesc *GetMethodDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final;
  const EventDesc *GetEventDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pThis->Component::GetEventDesc(_name, enumerateFlags); }
  const StaticFuncDesc *GetStaticFuncDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pThis->Component::GetStaticFuncDesc(_name, enumerateFlags); }

protected:
  VarComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~VarComponent();

  Variant::VarMap map;
};

}

#endif // _EP_VARCOMPONENT_H
