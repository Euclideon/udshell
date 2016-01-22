#pragma once
#if !defined(_EP_INODE_HPP)
#define _EP_INODE_HPP

#include "ep/cpp/component/component.h"
#include "ep/c/input.h"

namespace ep
{

SHARED_CLASS(RenderScene);
SHARED_CLASS(Node);

class INode
{
public:
  virtual void SetMatrix(const Double4x4 &mat) = 0;
  virtual const Double4x4& GetMatrix() const = 0;

  virtual void SetPosition(const Double3 &pos) = 0;
  virtual const Double3& GetPosition() const = 0;

  virtual NodeRef Parent() const = 0;
  virtual const Slice<NodeRef> Children() const = 0;

  virtual void AddChild(NodeRef c) = 0;
  virtual void RemoveChild(NodeRef c) = 0;

  virtual void Detach() = 0;

  virtual void CalculateWorldMatrix(Double4x4 *pMatrix) const = 0;

  virtual Variant Save() const = 0;
protected:
  virtual bool InputEvent(const epInputEvent &ev) = 0;
  virtual bool Update(double timeStep) = 0;
  virtual epResult Render(RenderScene &spScene, const Double4x4 &mat) = 0; // TODO Get rid of epResult
};

} // namespace ep

#endif // EPNODE_H
