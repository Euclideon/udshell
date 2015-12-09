#pragma once
#ifndef UD_VIEWPORT_H
#define UD_VIEWPORT_H

#include "ui.h"

namespace ep
{

SHARED_CLASS(Viewport);

class Viewport : public UIComponent
{
  EP_DECLARE_COMPONENT(Viewport, UIComponent, EPKERNEL_PLUGINVERSION, "Viewport desc...")
public:

protected:
  Viewport(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~Viewport();

  epResult CreateInternal(Variant::VarMap initParams);
  void DestroyInternal();
};

} // namespace ep

#endif // UD_VIEWPORT_H
