#pragma once
#ifndef _EP_TEXT_H
#define _EP_TEXT_H

#include "components/resources/buffer.h"

namespace ep
{

SHARED_CLASS(Buffer);

class Text : public Buffer
{
public:
  EP_COMPONENT(Text);

  Variant ParseXml() const;
  Variant ParseJson() const;

  void CopyBuffer(String text) { Buffer::CopyBuffer(text); }

protected:
  Text(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Buffer(pType, pKernel, uid, initParams) {}
  virtual ~Text() {}
};

} // namespace ep

#endif // _EP_TEXT_H
