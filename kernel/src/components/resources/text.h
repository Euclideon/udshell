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
  Variant ParseJson() const; // TODO: Implement this?

  static Variant XMLMapToComponentParams(Variant node);
  static Variant ComponentParamsToXMLMap(Variant map);

  void CopyBuffer(String text) { Buffer::CopyBuffer(text); }

  // TODO: move this somewhere else
  static uint32_t GetLineNumberFromByteIndex(String buffer, size_t index);

protected:
  Text(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Buffer(pType, pKernel, uid, initParams) {}
  virtual ~Text() {}

  void FormatXmlElement(StreamRef spOut, Variant::VarMap element, int depth);

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(ParseXml, "Parse XML formatted text in buffer into a heirarchical structure of KeyValuePairs"),
      EP_MAKE_METHOD(FormatXml, "Format a heirarchical structure of KeyValuePairs as XML text"),
    };
  }
  static Array<const StaticFuncInfo> GetStaticFuncs()
  {
    return{
      EP_MAKE_STATICFUNC(XMLMapToComponentParams, "Convert a map representing a heirarchy of XML elements into 'Component Params' format, i.e. suitable as InitParams and for returning from Save()"),
      EP_MAKE_STATICFUNC(ComponentParamsToXMLMap, "Convert a map in 'Component Params' format into a map representing a heirarchy of XML elements"),
    };
  }
};

} // namespace ep

#endif // _EP_TEXT_H
