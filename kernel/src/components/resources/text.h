#pragma once
#ifndef _EP_TEXT_H
#define _EP_TEXT_H

#include "components/resources/buffer.h"

namespace kernel
{

SHARED_CLASS(Buffer);

class Text : public Buffer
{
public:
  EP_COMPONENT(Text);

  Variant ParseXml();
  void FormatXml(Variant root);
  Variant ParseJson() const;

  void CopyBuffer(String text) { Buffer::CopyBuffer(text); }

protected:
  Text(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Buffer(pType, pKernel, uid, initParams) {}
  virtual ~Text() {}

  void FormatXmlElement(StreamRef spOut, KeyValuePair element, int depth);
};

} // namespace kernel

#endif // _EP_TEXT_H
