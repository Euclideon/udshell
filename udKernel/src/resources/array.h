#pragma once
#ifndef _UD_ARRAY_H
#define _UD_ARRAY_H

#include "resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Array);

class ArrayBuffer : public Resource
{
public:

protected:
  template<typename T>
  friend class udSharedPtr;

  size_t GetElementSize() const;

  size_t GetNumDimensions() const;

  size_t GetLength() const;
  udSlice<size_t> GetDimensions() const;

  void *GetBuffer() const;

private:
  ArrayBuffer() : Resource(ResourceType::Array) {};
  virtual ~ArrayBuffer();

};

} // namespace ud

#endif // _UD_ARRAY_H
