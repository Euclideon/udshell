#pragma once
#ifndef _UD_BUFFER_H
#define _UD_BUFFER_H

#include "resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Buffer);

class Buffer : public Resource
{
public:

protected:
  template<typename T>
  friend class udSharedPtr;

  void *GetBuffer() const;
  size_t GetBufferSize() const;

  void SetBuffer(void *pBuffer, size_t size);

private:
  Buffer() : Resource(ResourceType::Buffer) {};
  virtual ~Buffer();

};

} // namespace ud

#endif // _UD_BUFFER_H
