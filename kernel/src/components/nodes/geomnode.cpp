#include "geomnode.h"
#include "renderscene.h"
#include "renderresource.h"
#include "kernel.h"
#include "components/resources/model.h"

namespace ep
{

ComponentDesc GeomNode::descriptor =
{
  &Node::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "geomnode",    // id
  "GeomNode",  // displayName
  "Is a geometry node", // description

//  Slice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
//  Slice<CMethodDesc>(methods, UDARRAYSIZE(methods)) // methods
};

epResult GeomNode::Render(RenderSceneRef &spScene, const udDouble4x4 &mat)
{
  GeomJob &job = spScene->geom.pushBack();

  job.matrix = mat;

  MaterialRef spMat = spModel->GetMaterial();

  job.spProgram = spMat->GetRenderProgram();
  job.spVertexFormat = spModel->GetRenderVertexFormat(job.spProgram);

  // for each shader stream...
/*
  for (int i = 0; i < Model::ArrayType::Max; ++i)
  {
    ArrayBufferRef spArray = spModel->GetArray(i);
    ArrayBuffer::RenderResourceType rt = i == Model::ArrayType::Indices ? ArrayBuffer::RenderResourceType::IndexArray : ArrayBuffer::RenderResourceType::VertexArray;
    job.arrays[i] = spArray ? spArray->GetRenderResource(rt) : nullptr;
  }

  for (int i = 0; i < 8; ++i)
  {
    ArrayBufferRef spArray = spModel->GetArray(i);
    job.textures[i] = spArray ? spArray->GetRenderResource(ArrayBuffer::RenderResourceType::Texture) : nullptr;
  }
*/

  return epR_Success;
}

} // namespace ep
