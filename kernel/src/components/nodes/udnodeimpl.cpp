#include "components/resources/udmodelimpl.h"
#include "udnodeimpl.h"
#include "udOctree.h"
#include "renderscene.h"
#include "ep/cpp/kernel.h"
#include "udRender.h"
#include "components/datasources/udsource.h"

namespace ep {

Array<const PropertyInfo> UDNode::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("udModel", GetUDModel, SetUDModel, "UDModel instance", nullptr, 0),
  };
}

void UDNodeImpl::SetUDModel(UDModelRef _spModel)
{
  if (spModel)
    spModel->Changed.Unsubscribe(pInstance->Changed);

  spModel = _spModel;

  if (spModel)
    spModel->Changed.Subscribe(pInstance->Changed);
}

void UDNodeImpl::Render(RenderScene &spScene, const Double4x4 &mat)
{
  if (spModel)
  {
    UDRenderJob &job = spScene.ud.pushBack();
    job.spModel = spModel;
    job.matrix = mat;
    job.udNodePtr = NodeRef(pInstance);
  }
}

} // namespace ep
