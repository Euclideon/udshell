#pragma once
#ifndef _UD_TEXT_H
#define _UD_TEXT_H

#include "components/resources/buffer.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Buffer);

class Text : public Buffer
{
public:
  UD_COMPONENT(Text);

  udVariant ParseXml() const;
  udVariant ParseJson() const;

  void SetBuffer(udString text) { Buffer::SetBuffer(text.ptr, text.length); }

protected:
  Text(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Buffer(pType, pKernel, uid, initParams) {}
  virtual ~Text();
};

} // namespace ud

#endif // _UD_TEXT_H
