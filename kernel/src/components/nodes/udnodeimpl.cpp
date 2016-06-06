#include "components/resources/udmodelimpl.h"
#include "udnodeimpl.h"
#include "udOctree.h"
#include "renderscene.h"
#include "ep/cpp/kernel.h"
#include "udRender.h"
#include "components/datasources/udsource.h"

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
    job.matrix = mat;
  }
}

} // namespace ep
