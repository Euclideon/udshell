#pragma once
#ifndef _EP_TEXTNODEIMPL_HPP
#define _EP_TEXTNODEIMPL_HPP

#include "ep/cpp/component/node/textnode.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/resource/model.h"

namespace ep {

class TextNodeImpl : public BaseImpl<TextNode, ITextNode>
{
public:
  TextNodeImpl(Component *pInstance, Variant::VarMap initParams);

  SharedString getText() const override final { return text; }
  void setText(SharedString _text) override final
  {
    if (!text.eq(_text))
    {
      text = _text;
      spTextImage = nullptr;
      pInstance->changed.signal();
    }
  }

  double getHeight() const override final { return height; }
  void setHeight(double _height) override final
  {
    if (height != _height)
    {
      height = _height;
      spTextImage = nullptr;
      pInstance->changed.signal();
    }
  }

  FontRef getFont() const override final { return spFont; }
  void setFont(FontRef _spFont) override final
  {
    if (spFont != _spFont)
    {
      spFont = _spFont;
      spTextImage = nullptr;
      pInstance->changed.signal();
    }
  }

  Justification getJustification() const override final { return justification; }
  void setJustification(Justification _justification) override final
  {
    if (justification != _justification)
    {
      justification = _justification;
      pInstance->changed.signal();
    }
  }

  double getOutlineWidth() const override final { return outlineWidth; }
  void setOutlineWidth(double _outlineWidth) override final
  {
    if (outlineWidth != _outlineWidth)
    {
      outlineWidth = _outlineWidth;
      spTextImage = nullptr;
      pInstance->changed.signal();
    }
  }

  Float3 getColor() const override final { return color; }
  void setColor(Float3 _color) override final
  {
    if (color != _color)
    {
      color = _color;
      spTextImage = nullptr;
      pInstance->changed.signal();
    }
  }
  Float3 getOutlineColor() const override final { return outlineColor; }
  void setOutlineColor(Float3 _color) override final
  {
    if (outlineColor != _color)
    {
      outlineColor = _color;
      spTextImage = nullptr;
      pInstance->changed.signal();
    }
  }

protected:
  void render(RenderScene &spScene, const Double4x4 &mat) override final;

  void createStaticResources();

  FontRef spFont;
  SharedString text;
  double height = 16.0;
  double outlineWidth = 1.0;
  Float3 color = Float3::one();
  Float3 outlineColor = Float3::zero();
  Justification justification = Justification::TopLeft;
  ArrayBufferRef spTextImage;
  MaterialRef spTextMat;

  SharedPtr<RefCounted> spVertexFormatCache = nullptr;
  void RetainShaderInputConfig(SharedPtr<RefCounted> sp) { spVertexFormatCache = sp; }

  static ArrayBufferRef spQuadVerts;
  static ShaderRef spTextVS;
  static ShaderRef spTextPS;
};

} // namespace ep

#endif // _EP_TEXTNODEIMPL_HPP
