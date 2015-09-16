#pragma once
#ifndef UD_PROJECT_H
#define UD_PROJECT_H

#include "component.h"

namespace ud
{
PROTOTYPE_COMPONENT(Project);

class Project : public Component
{
public:
  UD_COMPONENT(Project);

protected:
  Project(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);

};

} //namespace ud
#endif // UD_PROJECT_H
