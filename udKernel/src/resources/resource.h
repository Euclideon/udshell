#pragma once
#if !defined(_UD_RESOURCE_H)
#define _UD_RESOURCE_H

#include "util/udvariant.h"

namespace ud
{

SHARED_CLASS(DataSource);

enum class ResourceType : int
{
  Array,
  SparseArray,
  UD,
  Shader,
  Text,
};

class Resource : public udRefCounted
{
public:
  int Type() const { return (int)type; }

  virtual udVariant GetMetadata(udString key) const { return udVariant(); }

protected:
  Resource(ResourceType type, DataSourceRef dataSource = nullptr)
    : type(type), source(dataSource) {}
  Resource() = delete;

  ResourceType type;

  DataSourceRef source;
};
SHARED_CLASS(Resource);

}

#endif // _UD_RESOURCE_H
