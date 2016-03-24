#include "ep/cpp/component/resource/model.h"
#include "modelimpl.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> Model::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(Material, "The Model's Material", nullptr, 0),
    EP_MAKE_PROPERTY(IndexArray, "The Model's index buffer", nullptr, 0),
  };
}
Array<const MethodInfo> Model::GetMethods() const
{
  return{
// TODO: why no this bind?
//    EP_MAKE_METHOD(SetVertexArray, "Set the Vertex buffer given an array of vertices and an array of attributes"), // TODO: Add this when we support passing slice to function
  };
}

void ModelImpl::SetVertexArray(ArrayBufferRef spVertices, Slice<const SharedString> attributeNames)
{
//  String type = spVertices->GetType();
  // TODO: assert that type has the same number of elements as `attributeNames.length`.

  auto &a = vertexArrays.pushBack();
  a.spArray = spVertices;
  a.attributes = attributeNames;
}

} // namespace ep
