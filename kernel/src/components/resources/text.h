#pragma once
#ifndef _EP_TEXT_H
#define _EP_TEXT_H

#include "components/resources/buffer.h"
#include "components/stream.h"

namespace ep
{

SHARED_CLASS(Buffer);

class Text : public Buffer
{
  EP_DECLARE_COMPONENT(Text, Buffer, EPKERNEL_PLUGINVERSION, "Text resource")
public:

  Variant ParseXml();
  void FormatXml(Variant root);
  Variant ParseJson() const;

  static Variant XMLNodeToMap(Variant node);
  static Variant MapToXMLNode(Variant map);

  void CopyBuffer(String text) { Buffer::CopyBuffer(text); }

  // TODO: move this somewhere else
  static uint32_t GetLineNumberFromByteIndex(String buffer, size_t index);

protected:
  Text(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Buffer(pType, pKernel, uid, initParams) {}
  virtual ~Text() {}

  void FormatXmlElement(StreamRef spOut, Variant::VarMap element, int depth);

  static Variant KVPToXMLNode(KeyValuePair kvp);

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(ParseXml, "Parse XML formatted text in buffer into a heirarchical structure of KeyValuePairs"),
      EP_MAKE_METHOD(FormatXml, "Format a heirarchical structure of KeyValuePairs as XML text"),
    };
  }
};

} // namespace ep

#endif // _EP_TEXT_H
