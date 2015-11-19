
#include "scene.h"
#include "view.h"
#include "kernel.h"
#include "renderscene.h"

namespace kernel
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
      "makedirty", // id
      "Force a dirty signal", // description
    },
    &Scene::MakeDirty, // method
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

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "scene",      // id
  "Scene",    // displayName
  "Is a scene", // description

  Slice<CPropertyDesc>(props, EPARRAYSIZE(props)), // propeties
  Slice<CMethodDesc>(methods, EPARRAYSIZE(methods)), // methods
  Slice<CEventDesc>(events, EPARRAYSIZE(events)) // events
};


bool Scene::InputEvent(const epInputEvent &ev)
{
  // do anything here?
  //...

  // pass input to the hierarchy...
  return rootNode->InputEvent(ev);
}

void Scene::Update(double timeDelta)
{
  // do anything here?
  //...

  // update the hierarchy...
  if (rootNode->Update(timeDelta))
    MakeDirty();
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

epResult Scene::SetRenderModels(struct udRenderModel models[], size_t numModels)
{
  for (size_t i = 0; i < numModels; ++i)
    renderModels[i] = models[i];
  numRenderModels = numModels;

  return epR_Success;
}

Scene::Scene(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams) :
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
}  // namespace kernel
