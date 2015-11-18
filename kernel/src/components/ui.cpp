#include "ui.h"

namespace ep
{

static CPropertyDesc props[] =
{
  {
    {
      "uihandle",
      "UIHandle",
      "Platform specific UI handle"
    },
    &UIComponent::GetUIHandle,
    nullptr
  }
};

ComponentDesc UIComponent::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION,     // epVersion
  EPSHELL_PLUGINVERSION,  // pluginVersion

  "ui",                // id
  "UI",                // displayName
  "Is a UI component", // description

  Slice<CPropertyDesc>(props, EPARRAYSIZE(props)), // propeties
};

UIComponent::UIComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != epR_Success)
    throw epR_Failure;
}

UIComponent::~UIComponent()
{
  DestroyInternal();
}

} // namespace ep
