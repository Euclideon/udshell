#include "udnode.h"
#include "udOctree.h"
#include "renderscene.h"
#include "kernel.h"
#include "udRender.h"
#include "components/datasources/uddatasource.h"

namespace kernel
{

epResult UDNode::Render(RenderSceneRef &spScene, const Double4x4 &mat)
{
  UDJob &job = spScene->ud.pushBack();
  memset(&job, 0, sizeof(job));

  job.renderState = spModel->GetUDRenderState();
  job.renderState.matrix = Mul(mat, job.renderState.matrix);
  job.renderState.pWorldMatrixD = job.renderState.matrix.a;

  return epR_Success;
}

} // namespace kernel
