#pragma once
#if !defined(_UD_RESOURCE_H)
#define _UD_RESOURCE_H

#include "components/component.h"

namespace ud
{

SHARED_CLASS(DataSource);

enum class ResourceType : int
{
  Buffer,
  Array,
  SparseArray,
  UD,
  Shader,
  Text,
  Model,
};

class Resource : public Component
{
public:
  UD_COMPONENT(Resource);

  int Type() const { return (int)type; }

  virtual udVariant GetMetadata(udString key) const { return udVariant(); }

protected:
  Resource(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Component(pType, pKernel, uid, initParams) {}

  ResourceType type;

  DataSourceRef source = nullptr;
};
SHARED_CLASS(Resource);

}

#endif // _UD_RESOURCE_H
