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
    EP_MAKE_PROPERTY("udModel", getUDModel, setUDModel, "UDModel instance", nullptr, 0),
  };
}

void UDNodeImpl::SetUDModel(UDModelRef _spModel)
{
  if (spModel)
    spModel->changed.unsubscribe(pInstance->changed);

  spModel = _spModel;

  if (spModel)
    spModel->changed.subscribe(pInstance->changed);
}

void UDNodeImpl::Render(RenderScene &spScene, const Double4x4 &mat)
{
  if (spModel)
  {
    UDRenderJob &job = spScene.ud.pushBack();
    job.spModel = spModel;
    const Double3 &p = spModel->getPivot();
    job.matrix = Double4x4::create( 1.0,  0.0,  0.0, 0.0,
                                    0.0,  1.0,  0.0, 0.0,
                                    0.0,  0.0,  1.0, 0.0,
                                   -p.x, -p.y, -p.z, 1.0) * mat;
    job.udNodePtr = NodeRef(pInstance);
  }
}

} // namespace ep
