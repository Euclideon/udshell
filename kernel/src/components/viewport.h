#pragma once
#ifndef UD_VIEWPORT_H
#define UD_VIEWPORT_H

#include "ui.h"

namespace ep
{

PROTOTYPE_COMPONENT(Viewport);

class Viewport : public UIComponent
{
public:
  EP_COMPONENT(Viewport);

protected:
  Viewport(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);
  virtual ~Viewport();

  epResult CreateInternal(epInitParams initParams);
  void DestroyInternal();
};

} // namespace ep

#endif // UD_VIEWPORT_H
