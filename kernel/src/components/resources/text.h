#pragma once
#ifndef _UD_TEXT_H
#define _UD_TEXT_H

#include "components/resources/buffer.h"
#include "ep/epstring.h"

namespace ud
{

SHARED_CLASS(Buffer);

class Text : public Buffer
{
public:
  UD_COMPONENT(Text);

  udVariant ParseXml() const;
  udVariant ParseJson() const;

  void CopyBuffer(udString text) { Buffer::CopyBuffer(text); }

protected:
  Text(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Buffer(pType, pKernel, uid, initParams) {}
  virtual ~Text() {}
};

} // namespace ud

#endif // _UD_TEXT_H
