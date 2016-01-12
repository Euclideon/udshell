#include "ep/cpp/component/activity.h"

namespace ep
{

Activity::Activity(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  pImpl = CreateImpl(initParams);
}

ComponentRef Activity::GetUI() const
{
  return pImpl->GetUI();
}

void Activity::SetUI(ComponentRef ui)
{
  pImpl->SetUI(ui);
}

void Activity::Activate()
{
  pImpl->Activate();
}
void Activity::Deactivate()
{
  pImpl->Deactivate();
}

Variant Activity::Save() const
{
  return pImpl->Save();
}

} // namespace ep
