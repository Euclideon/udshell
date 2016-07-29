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

  Variant save() const override;

protected:
  ~ComponentGlue();

  void receiveMessage(String message, String sender, const Variant &data) override;

  const PropertyDesc *getPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final { return spInstance->getPropertyDesc(_name, enumerateFlags); }
  const MethodDesc *getMethodDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final { return spInstance->getMethodDesc(_name, enumerateFlags); }
  const EventDesc *getEventDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final  { return spInstance->getEventDesc(_name, enumerateFlags); }
  const StaticFuncDesc *getStaticFuncDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final  { return spInstance->getStaticFuncDesc(_name, enumerateFlags); }

  ComponentRef spInstance;

  VarDelegate _save;
  VarDelegate _receiveMessage;
};

} // namespace ep

#endif // _EP_COMPONENTGLUE_HPP
