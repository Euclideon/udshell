#pragma once
#ifndef UD_VIEWPORT_H
#define UD_VIEWPORT_H

#include "ui.h"

namespace ud
{

PROTOTYPE_COMPONENT(Viewport);

class Viewport : public UIComponent
{
public:
  UD_COMPONENT(Viewport);

protected:
  Viewport(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
  virtual ~Viewport();

  udResult CreateInternal(udInitParams initParams);
  void DestroyInternal();
};

} // namespace ud

#endif // UD_VIEWPORT_H
