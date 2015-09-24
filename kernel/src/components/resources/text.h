#pragma once
#ifndef _EP_TEXT_H
#define _EP_TEXT_H

#include "components/resources/buffer.h"
#include "ep/epstring.h"

namespace ep
{

SHARED_CLASS(Buffer);

class Text : public Buffer
{
public:
  EP_COMPONENT(Text);

  epVariant ParseXml() const;
  epVariant ParseJson() const;

  void CopyBuffer(epString text) { Buffer::CopyBuffer(text); }

protected:
  Text(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Buffer(pType, pKernel, uid, initParams) {}
  virtual ~Text() {}
};

} // namespace ep

#endif // _EP_TEXT_H
