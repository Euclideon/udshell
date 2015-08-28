
#include "scene.h"
#include "view.h"
#include "kernel.h"
#include "renderscene.h"

namespace ud
{

static CPropertyDesc props[] =
{
  {
    {
      "rootnode", // id
      "RootNode", // displayName
      "Scene root node", // description
    },
    &Scene::GetRootNode, // getter
    nullptr  // setter
  }
};
static CMethodDesc methods[] =
{
  {
    {
      "forcedirty", // id
      "Force a dirty signal", // description
    },
    &Scene::ForceDirty, // method
  }
};
static CEventDesc events[] =
{
  {
    {
      "dirty", // id
      "Dirty", // displayName
      "Scene dirty event", // description
    },
    &Scene::Dirty
  }
};
ComponentDesc Scene::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "scene",      // id
  "Scene",    // displayName
  "Is a scene", // description

  udSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // propeties
  udSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  udSlice<CEventDesc>(events, UDARRAYSIZE(events)) // events
};


udResult Scene::InputEvent(const udInputEvent &ev)
{
  // do anything here?
  //...

  // pass input to the hierarchy...
  return rootNode->InputEvent(ev);
}

udResult Scene::Update(double timeDelta)
{
  // do anything here?
  //...

  // update the hierarchy...
  return rootNode->Update(timeDelta);
}

RenderSceneRef Scene::GetRenderScene()
{
  if (!bDirty)
    return spCache;

  spCache = RenderSceneRef::create();

  // build scene
  rootNode->Render(spCache, rootNode->GetMatrix());

  bDirty = false;

  return spCache;
}

udResult Scene::SetRenderModels(struct udRenderModel models[], size_t numModels)
{
  for (size_t i = 0; i < numModels; ++i)
    renderModels[i] = models[i];
  numRenderModels = numModels;

  return udR_Success;
}

Scene::Scene(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams) :
  Component(pType, pKernel, uid, initParams)
{
  timeStep = 1.0 / 30.0;
  rootNode = pKernel->CreateComponent<Node>();

  memset(&renderModels, 0, sizeof(renderModels));
  numRenderModels = 0;
}

Scene::~Scene()
{

}
}  // namespace ud
