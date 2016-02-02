#include "udnodeimpl.h"
#include "udOctree.h"
#include "renderscene.h"
#include "kernel.h"
#include "udRender.h"
#include "components/datasources/uddatasource.h"
#include "ep/cpp/component/resource/udmodel.h"

namespace ep
{

void UDNodeImpl::Render(RenderScene &spScene, const Double4x4 &mat)
{
  UDRenderJob &job = spScene.ud.pushBack();
  job.spModel = spModel;
  job.renderState = spModel->GetUDRenderState();
  job.renderState.matrix = Mul(mat, job.renderState.matrix);
}

} // namespace ep
