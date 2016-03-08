#pragma once
#if !defined(_EP_COMPONENTGLUE_HPP)
#define _EP_COMPONENTGLUE_HPP

#include "ep/cpp/component/component.h"

namespace ep {

struct DynamicComponentDesc : public ComponentDescInl
{
  using NewInstanceFunc = Delegate<ComponentRef(ComponentRef instance, Variant::VarMap initParams)>;
  NewInstanceFunc newInstance;
};

// component API
class ComponentGlue final : public Component
{
public:
  ComponentGlue(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams);

  Variant Save() const override;

protected:
  ~ComponentGlue();

  void InitComplete() override;
  void ReceiveMessage(String message, String sender, const Variant &data) override;

  ComponentRef spInstance;

  MethodShim::DelegateType save;
  MethodShim::DelegateType initComplete;
  MethodShim::DelegateType receiveMessage;
};

} // namespace ep

#endif // _EP_COMPONENTGLUE_HPP
