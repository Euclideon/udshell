#include "ep/cpp/component/resource/model.h"
#include "modelimpl.h"
#include "renderresource.h"
#include "kernel.h"

namespace ep {

void ModelImpl::SetVertexArray(ArrayBufferRef spVertices, Slice<const SharedString> attributeNames)
{
//  String type = spVertices->GetType();
  // TODO: assert that type has the same number of elements as `attributeNames.length`.

  auto &a = vertexArrays.pushBack();
  a.spArray = spVertices;
  a.attributes = attributeNames;
}

} // namespace ep
