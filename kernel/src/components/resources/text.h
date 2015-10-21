#pragma once
#ifndef _EP_TEXT_H
#define _EP_TEXT_H

#include "components/resources/buffer.h"
#include "rapidxml.hpp"

namespace ep
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

  KeyValuePair ParseXMLNode(rapidxml::xml_node<> *node) const;
  void FormatXmlElement(StreamRef spOut, KeyValuePair element, int depth);
};

} // namespace ep

#endif // _EP_TEXT_H
