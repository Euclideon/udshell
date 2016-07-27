#pragma once
#ifndef _EP_TEXT_HPP
#define _EP_TEXT_HPP

#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/component/stream.h"
#include "ep/cpp/internal/i/itext.h"

namespace ep {

SHARED_CLASS(Text);

class Text : public Buffer
{
  EP_DECLARE_COMPONENT_WITH_STATIC_IMPL(ep, Text, IText, ITextStatic, Buffer, EPKERNEL_PLUGINVERSION, "Text resource", 0)
public:

  Variant ParseXml() { return pImpl->ParseXml(); }
  void FormatXml(Variant root) { pImpl->FormatXml(root); }
  Variant ParseJson() { return pImpl->ParseJson(); }
  void FormatJson(Variant root) { pImpl->FormatJson(root); }

  static Variant XMLMapToComponentParams(Variant node) { return GetStaticImpl()->XMLMapToComponentParams(node); }
  static Variant ComponentParamsToXMLMap(Variant map) { return GetStaticImpl()->ComponentParamsToXMLMap(map); }

  void CopyBuffer(String text) { Buffer::copyBuffer(text); }

  Slice<char> Map()
  {
    Slice<void> _buffer = Buffer::map();
    return Slice<char>((char*)_buffer.ptr, _buffer.length);
  }
  String MapForRead()
  {
    Slice<const void> buffer = Buffer::mapForRead();
    return String((const char*)buffer.ptr, buffer.length);
  }

  // TODO: move this somewhere else!
  static uint32_t GetLineNumberFromByteIndex(String buffer, size_t index) { return GetStaticImpl()->GetLineNumberFromByteIndex(buffer, index); }

protected:
  Text(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Buffer(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  Array<const MethodInfo> getMethods() const;
  Array<const StaticFuncInfo> getStaticFuncs() const;
};

} // namespace ep

#endif // _EP_TEXT_HPP
