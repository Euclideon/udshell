#pragma once
#ifndef _EP_TEXT_H
#define _EP_TEXT_H

#include "components/resources/buffer.h"

namespace kernel
{

SHARED_CLASS(Buffer);

class Text : public Buffer
{
  EP_DECLARE_COMPONENT(Text, Buffer, EPKERNEL_PLUGINVERSION, "Text resource")
public:

  Variant ParseXml();
  void FormatXml(Variant root);
  Variant ParseJson() const;

  void CopyBuffer(String text) { Buffer::CopyBuffer(text); }

protected:
  Text(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Buffer(pType, pKernel, uid, initParams) {}
  virtual ~Text() {}

  void FormatXmlElement(StreamRef spOut, KeyValuePair element, int depth);

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(ParseXml, "Parse XML formatted text in buffer into a heirarchical structure of KeyValuePairs"),
      EP_MAKE_METHOD(FormatXml, "Format a heirarchical structure of KeyValuePairs as XML text"),
    };
  }
};

} // namespace kernel

#endif // _EP_TEXT_H
