#include "scenenodeimpl.h"
#include "components/sceneimpl.h"
#include "renderscene.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/scene.h"

namespace ep {

Array<const PropertyInfo> SceneNode::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("scene", getScene, setScene, "Scene instance", nullptr, 0),
  };
}

void SceneNodeImpl::Render(RenderScene &spRenderScene, const Double4x4 &mat)
{
  if (spScene)
    spScene->getRootNode()->doRender(spRenderScene, mat);

  // TODO: call Node::Render()?
}

} // namespace ep
