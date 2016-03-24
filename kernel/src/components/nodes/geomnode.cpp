#include "geomnode.h"
#include "renderscene.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/resource/material.h"

namespace ep {

Array<const PropertyInfo> GeomNode::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(Model, "The Node's Model", nullptr, 0),
  };
}

void GeomNode::Render(RenderScene &spScene, const Double4x4 &mat)
{
  GeomRenderJob &job = spScene.geom.pushBack();

  job.matrix = mat;

  MaterialRef spMat = spModel->GetMaterial();

#if 0 // TODO : For Manu
  job.spProgram = spMat->GetImpl<MaterialImpl>()->GetRenderProgram();
  job.spVertexFormat = spModel->GetRenderVertexFormat(job.spProgram);
#endif // 0

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
}

} // namespace ep
