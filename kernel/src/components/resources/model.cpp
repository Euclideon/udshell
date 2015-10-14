#include "model.h"
#include "renderresource.h"
#include "kernel.h"

namespace ep
{

ComponentDesc Model::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "model", // id
  "Model", // displayName
  "Model resource", // description
};

RenderVertexFormatRef Model::GetRenderVertexFormat(RenderShaderProgramRef spShaderProgram)
{
  if (!spRenderVertexFormat)
  {
    EPASSERT(false, "TODO");

    // get the attributes from the material

    // create a descriptor that maps vertex arrays to shader attributes

//    VertexFormatDescriptor *pFormat = new VertexFormatDescriptor(pKernel->GetRenderer(), spArrays);
//    spRenderVertexFormat = VertexFormatDescriptorRef(pFormat);
  }
  return spRenderVertexFormat;
}

void Model::SetVertexArray(ArrayBufferRef spVertices, Slice<const SharedString> attributeNames)
{
//  String type = spVertices->GetType();
  // TODO: assert that type has the same number of elements as `attributeNames.length`.

  auto &a = vertexArrays.pushBack();
  a.spArray = spVertices;
  a.attributes = attributeNames;
}

} // namespace ep
