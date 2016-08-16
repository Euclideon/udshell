#pragma once
#ifndef _EP_TEXTNODE_HPP
#define _EP_TEXTNODE_HPP

#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/itextnode.h"
#include "ep/cpp/component/resource/font.h"

namespace ep {

SHARED_CLASS(TextNode);

class TextNode : public Node
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, TextNode, ITextNode, Node, EPKERNEL_PLUGINVERSION, "Render text within a scene", 0)
public:

  SharedString getText() const { return pImpl->getText(); }
  void setText(SharedString _text) { pImpl->setText(_text); }

  FontRef getFont() const { return pImpl->getFont(); }
  void setFont(FontRef _font) { pImpl->setFont(_font); }

  double getHeight() const { return pImpl->getHeight(); }
  void setHeight(double _height) { pImpl->setHeight(_height); }

  Justification getJustification() const { return pImpl->getJustification(); }
  void setJustification(Justification _justification) { pImpl->setJustification(_justification); }

  double getOutlineWidth() const { return pImpl->getOutlineWidth(); }
  void setOutlineWidth(double _outlineWidth) { pImpl->setOutlineWidth(_outlineWidth); }

  Float3 getColor() const { return pImpl->getColor(); }
  void setColor(Float3 _color) { pImpl->setColor(_color); }
  Float3 getOutlineColor() const { return pImpl->getOutlineColor(); }
  void setOutlineColor(Float3 _color) { pImpl->setOutlineColor(_color); }

protected:
  TextNode(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  void render(RenderScene &spScene, const Double4x4 &mat) override { pImpl->render(spScene, mat); }

private:
  Array<const PropertyInfo> getProperties() const;
};

} // namespace ep

#endif // _EP_TEXTNODE_HPP
