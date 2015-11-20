#include "array.h"
#include "renderresource.h"
#include "kernel.h"

namespace kernel
{

RenderResourceRef ArrayBuffer::GetRenderResource(RenderResourceType type)
{
  if (!spRenderData)
  {
    if (type == RenderResourceType::Texture)
    {
      // TODO: we need to find texture type from the dimensions of the array buffer
      //       ... or we need 'type' to ask for the particular texture type that the shader expects
      RenderTexture *pRenderTexture = new RenderTexture(GetKernel().GetRenderer(), ArrayBufferRef(this), TextureUsage::_2D);
      spRenderData = RenderTextureRef(pRenderTexture);
    }
    else
    {
      RenderArray *pRenderArray = new RenderArray(GetKernel().GetRenderer(), ArrayBufferRef(this), type == RenderResourceType::IndexArray ? ArrayUsage::IndexData : ArrayUsage::VertexData);
      spRenderData = RenderArrayRef(pRenderArray);
    }
  }
  return spRenderData;
}

} // namespace kernel
