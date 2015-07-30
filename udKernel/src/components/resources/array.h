#pragma once
#ifndef _UD_ARRAYBUFFER_H
#define _UD_ARRAYBUFFER_H

#include "components/resources/buffer.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(ArrayBuffer);

class ArrayBuffer : public Buffer
{
public:
  UD_COMPONENT(ArrayBuffer);

  size_t GetElementSize() const;

  size_t GetNumDimensions() const;

  size_t GetLength() const;
  udSlice<size_t> GetDimensions() const;

protected:
  ArrayBuffer(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Buffer(pType, pKernel, uid, initParams) {}
  virtual ~ArrayBuffer();

  size_t dimensions;
  udSlice<size_t> shape;

  // element type description...
};

} // namespace ud

#endif // _UD_ARRAYBUFFER_H
