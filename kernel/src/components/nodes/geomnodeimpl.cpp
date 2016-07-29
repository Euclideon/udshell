#include "geomnodeimpl.h"
#include "renderscene.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/resource/material.h"
#include "components/resources/modelimpl.h"
#include "components/resources/materialimpl.h"


namespace ep {

Array<const PropertyInfo> GeomNode::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("model", getModel, setModel, "The Node's Model", nullptr, 0),
  };
}

void GeomNodeImpl::Render(RenderScene &spScene, const Double4x4 &mat)
{
  GeomRenderJob &job = spScene.geom.pushBack();
  job.matrix = mat;
  job.spMaterial = spModel->getMaterial();

  ModelImpl *pModelImpl = spModel->getImpl<ModelImpl>();
  job.vertexArrays = pModelImpl->vertexArrays;
  job.spIndices = pModelImpl->spIndices;
  job.renderList = pModelImpl->renderList;

  job.retainShaderInputConfig = Delegate<void(SharedPtr<RefCounted>)>(pModelImpl, &ModelImpl::RetainShaderInputConfig);
}

} // namespace ep
