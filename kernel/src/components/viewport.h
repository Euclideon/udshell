#pragma once
#ifndef UD_VIEWPORT_H
#define UD_VIEWPORT_H

#include "ui.h"

namespace kernel
{

PROTOTYPE_COMPONENT(Viewport);

class Viewport : public UIComponent
{
public:
  EP_COMPONENT(Viewport);

protected:
  Viewport(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  virtual ~Viewport();

  epResult CreateInternal(InitParams initParams);
  void DestroyInternal();
};

} // namespace kernel

#endif // UD_VIEWPORT_H
