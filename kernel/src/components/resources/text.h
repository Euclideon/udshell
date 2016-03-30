#pragma once
#ifndef _EP_TEXT_H
#define _EP_TEXT_H

#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/component/stream.h"

namespace ep {

SHARED_CLASS(Buffer);

class Text : public Buffer
{
  EP_DECLARE_COMPONENT(ep, Text, Buffer, EPKERNEL_PLUGINVERSION, "Text resource", 0)
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

  Array<const MethodInfo> GetMethods() const;
  Array<const StaticFuncInfo> GetStaticFuncs() const;
};

} // namespace ep

#endif // _EP_TEXT_H
