#include "ui.h"

namespace ud
{

// UICOMPONENT //////////////////////

ComponentDesc UIComponent::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION,     // udVersion
  UDSHELL_PLUGINVERSION,  // pluginVersion

  "ui",                // id
  "UI",                // displayName
  "Is a UI component", // description

  nullptr, // properties
  nullptr, // methods
  nullptr, // events

  nullptr, // pInit
  nullptr, // pCreate
};

// VIEWPORT //////////////////////

ComponentDesc Viewport::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION,     // udVersion
  UDSHELL_PLUGINVERSION,  // pluginVersion

  "viewport",             // id
  "Viewport",             // displayName
  "Is a viewport",        // description

  nullptr,                // properties
  nullptr,                // methods
  nullptr,                // events

  nullptr,                // pInit
  Viewport::Create,       // pCreate
};

// WINDOW //////////////////////

static CPropertyDesc windowProps[] =
{
  {
    {
      "ui", // id
      "UI", // displayName
      "Top level UI for the window", // description
    },
    &Window::GetUI, // getter
    &Window::SetUI, // setter
  }
};

ComponentDesc Window::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION,     // udVersion
  UDSHELL_PLUGINVERSION,  // pluginVersion

  "window",               // id
  "Window",               // displayName
  "Is a window",          // description

  nullptr,                // properties
  nullptr,                // methods
  nullptr,                // events

  nullptr,                // pInit
  Window::Create,         // pCreate
};

UIComponent::UIComponent(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  udString filename = initParams["file"].as<udString>();
  if (filename.empty())
  {
    LogError("Attempted to create ui component without source file");
    throw udR_Failure_;
  }
  if (CreateInternal(filename) != udR_Success)
    throw udR_Failure_;
}

UIComponent::~UIComponent()
{
  DestroyInternal();
}


// ---------------------------------------------------------------------------------------
Viewport::Viewport(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
  : UIComponent(pType, pKernel, uid, initParams)
{

}

// ---------------------------------------------------------------------------------------
Viewport::~Viewport()
{
}

// ---------------------------------------------------------------------------------------
void Window::SetUI(UIComponentRef spUIComponent)
{
  spTopLevelUI = spUIComponent;
  Refresh();
}

} // namespace ud
