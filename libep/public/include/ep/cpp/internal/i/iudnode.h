#pragma once
#if !defined(_EP_IUDNODE_HPP)
#define _EP_IUDNODE_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(UDModel);

class RenderScene;

class IUDNode
{
public:
  virtual UDModelRef GetUDModel() const = 0;
  virtual void SetUDModel(UDModelRef spModel) = 0;

  virtual Variant Save() const = 0;
protected:
  virtual epResult Render(RenderScene &spScene, const Double4x4 &mat) = 0;
};

} // namespace ep

#endif // EP_UDNODE_H
