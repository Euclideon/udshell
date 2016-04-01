#pragma once
#if !defined(_EP_COMPONENTGLUE_HPP)
#define _EP_COMPONENTGLUE_HPP

#include "ep/cpp/component/component.h"

namespace ep {

struct DynamicComponentDesc : public ComponentDescInl
{
  using NewInstanceFunc = Delegate<DynamicComponentRef(KernelRef spKernel, Variant::VarMap initParams)>;
  NewInstanceFunc newInstance;
  SharedPtr<RefCounted> userData;
};

// component API
class ComponentGlue final : public Component
{
public:
  ComponentGlue(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, ComponentRef spInstance, Variant::VarMap initParams);

  Variant Save() const override;

protected:
  ~ComponentGlue();

  void ReceiveMessage(String message, String sender, const Variant &data) override;

  const PropertyDesc *GetPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final { return spInstance->GetPropertyDesc(_name, enumerateFlags); }
  const MethodDesc *GetMethodDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final { return spInstance->GetMethodDesc(_name, enumerateFlags); }
  const EventDesc *GetEventDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final  { return spInstance->GetEventDesc(_name, enumerateFlags); }
  const StaticFuncDesc *GetStaticFuncDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final  { return spInstance->GetStaticFuncDesc(_name, enumerateFlags); }

  ComponentRef spInstance;

  VarDelegate save;
  VarDelegate receiveMessage;
};

} // namespace ep

#endif // _EP_COMPONENTGLUE_HPP
