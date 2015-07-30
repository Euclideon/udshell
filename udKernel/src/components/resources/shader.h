#pragma once
#ifndef _UD_SHADER_H
#define _UD_SHADER_H

#include "components/resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Shader);

class Shader : public Resource
{
public:
  UD_COMPONENT(Shader);

protected:
  Shader(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Shader();

};

} // namespace ud

#endif // _UD_SHADER_H
