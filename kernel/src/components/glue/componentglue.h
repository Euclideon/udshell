#pragma once
#if !defined(_EP_COMPONENTGLUE_HPP)
#define _EP_COMPONENTGLUE_HPP

#include "ep/cpp/component/component.h"

namespace ep {

struct DynamicComponentDesc : public ComponentDescInl
{
  using NewInstanceFunc = Delegate<DynamicComponentRef(Variant::VarMap initParams)>;
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

  ComponentRef spInstance;

  VarDelegate save;
  VarDelegate initComplete;
  VarDelegate receiveMessage;
};

} // namespace ep

#endif // _EP_COMPONENTGLUE_HPP
