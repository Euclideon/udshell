#pragma once
#ifndef EP_PROJECT_H
#define EP_PROJECT_H

#include "component.h"

namespace ep
{

PROTOTYPE_COMPONENT(Project);

class Project : public Component
{
public:
  EP_COMPONENT(Project);

protected:
  Project(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);

};

} //namespace ep

#endif // EP_PROJECT_H
