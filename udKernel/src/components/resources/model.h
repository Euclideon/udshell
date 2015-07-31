#pragma once
#ifndef _UD_MODEL_H
#define _UD_MODEL_H

#include "components/resources/resource.h"
#include "components/resources/array.h"
#include "components/resources/shader.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Model);

class Model : public Resource
{
public:
  UD_COMPONENT(Model);

  ArrayBufferRef GetVertexBuffer() const { return vertices; }

protected:
  Model(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Model();

  ArrayBufferRef vertices;
  ArrayBufferRef indices;
  ShaderRef shader;
};

} // namespace ud

#endif // _UD_MODEL_H
