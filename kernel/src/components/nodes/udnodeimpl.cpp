#include "udnodeimpl.h"
#include "udOctree.h"
#include "renderscene.h"
#include "ep/cpp/kernel.h"
#include "udRender.h"
#include "components/datasources/udsource.h"
#include "ep/cpp/component/resource/udmodel.h"

namespace ep {

Array<const PropertyInfo> UDNode::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(UDModel, "UDModel instance", nullptr, 0),
  };
}

void UDNodeImpl::Render(RenderScene &spScene, const Double4x4 &mat)
{
  if (spModel)
  {
    UDRenderJob &job = spScene.ud.pushBack();
    job.spModel = spModel;
    job.renderState = spModel->GetUDRenderState();
    job.renderState.matrix = Mul(mat, job.renderState.matrix);
  }
}

} // namespace ep
