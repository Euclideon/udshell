#pragma once
#ifndef GEOMNODE_H
#define GEOMNODE_H

#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/igeomnode.h"

#include "ep/cpp/component/resource/resource.h"

namespace ep
{

  SHARED_CLASS(Model);
  SHARED_CLASS(GeomNode);
  SHARED_CLASS(RenderScene);

  class GeomNode : public Node
  {
    EP_DECLARE_COMPONENT_WITH_IMPL(ep, GeomNode, IGeomNode, Node, EPKERNEL_PLUGINVERSION, "GeomNode desc...", 0)
  public:

    virtual ModelRef getModel() const { return pImpl->GetModel(); }
    virtual void setModel(ModelRef _spModel) { pImpl->SetModel(_spModel); }

  protected:
    GeomNode(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
      : Node(pType, pKernel, uid, initParams)
    {
      pImpl = createImpl(initParams);
    }

    void render(RenderScene &spScene, const Double4x4 &mat) override { pImpl->Render(spScene, mat); }

  private:
    Array<const PropertyInfo> getProperties() const;
  };

} // namespace ep

#endif // GEOMNODE_H
