#pragma once
#if !defined(_EP_ITEXTNODE_HPP)
#define _EP_ITEXTNODE_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(RenderScene);
SHARED_CLASS(Font);

EP_ENUM(Justification,
  TopLeft,
  TopCenter,
  TopRight,
  CenterLeft,
  Center,
  CenterRight,
  BottomLeft,
  BottomCenter,
  BottomRight,
);

class ITextNode
{
public:
  virtual SharedString getText() const = 0;
  virtual void setText(SharedString text) = 0;

  virtual FontRef getFont() const = 0;
  virtual void setFont(FontRef _font) = 0;

  virtual double getHeight() const = 0;
  virtual void setHeight(double _height) = 0;

  virtual Justification getJustification() const = 0;
  virtual void setJustification(Justification _justification) = 0;

  virtual double getOutlineWidth() const = 0;
  virtual void setOutlineWidth(double _outlineWidth) = 0;

  virtual Float3 getColor() const = 0;
  virtual void setColor(Float3 _color) = 0;
  virtual Float3 getOutlineColor() const = 0;
  virtual void setOutlineColor(Float3 _color) = 0;

protected:
  virtual void render(RenderScene &spScene, const Double4x4 &mat) = 0;
};

} // namespace ep

#endif // _EP_ITEXTNODE_HPP
