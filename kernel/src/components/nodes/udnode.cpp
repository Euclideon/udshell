#include "udnode.h"
#include "udOctree.h"
#include "renderscene.h"
#include "kernel.h"
#include "udRender.h"
#include "components/datasources/uddatasource.h"
#include "ep/cpp/component/resource/udmodel.h"

namespace ep
{

epResult UDNode::Render(RenderScene &spScene, const Double4x4 &mat)
{
  UDRenderJob &job = spScene.ud.pushBack();
  job.spModel = spModel;
  job.renderState = spModel->GetUDRenderState();
  job.renderState.matrix = Mul(mat, job.renderState.matrix);

  return epR_Success;
}

} // namespace ep
