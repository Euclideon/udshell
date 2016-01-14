#include "components/activities/activityimpl.h"
#include "components/uicomponentimpl.h"
namespace ep
{

ActivityImpl::~ActivityImpl()
{
}

ComponentRef ActivityImpl::GetUI() const
{
  return ui;
}

void ActivityImpl::SetUI(ComponentRef _ui)
{
  ui = component_cast<UIComponent>(_ui);
}

void ActivityImpl::Activate()
{
}

void ActivityImpl::Deactivate()
{
}

Variant ActivityImpl::Save() const
{
  return pInstance->InstanceSuper::Save();
}

} //
