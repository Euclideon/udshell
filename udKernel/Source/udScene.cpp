
#include "udScene.h"
#include "udView.h"
#include "udKernel.h"
#include "udRenderScene.h"


static const udPropertyDesc props[] =
{
  {
    "rootnode", // id
    "RootNode", // displayName
    "Scene root node", // description
    &udScene::GetRootNode, // getter
    nullptr, // setter
    udTypeDesc(udPropertyType::Integer) // type
  }
};
const udComponentDesc udScene::descriptor =
{
  &udComponent::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "scene",      // id
  "udScene",    // displayName
  "Is a scene", // description

  [](){ return udR_Success; },             // pInit
  udScene::Create, // pCreateInstance

  udSlice<const udPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};


udResult udScene::InputEvent(const udInputEvent &ev)
{
  // do anything here?
  //...

  // pass input to the hierarchy...
  return rootNode->InputEvent(ev);
}

udResult udScene::Update(double timeDelta)
{
  // do anything here?
  //...

  // update the hierarchy...
  return rootNode->Update(timeDelta);
}

udRenderSceneRef udScene::GetRenderScene()
{
  if (!bDirty)
    return spCache;

  spCache = udRenderSceneRef::create();

  // build scene
  rootNode->Render(spCache, rootNode->GetMatrix());

  bDirty = false;

  return spCache;
}

udResult udScene::SetRenderModels(struct udRenderModel models[], size_t numModels)
{
  for (size_t i = 0; i < numModels; ++i)
    renderModels[i] = models[i];
  numRenderModels = numModels;

  return udR_Success;
}

udScene::udScene(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams) :
  udComponent(pType, pKernel, uid, initParams)
{
  timeStep = 1.0 / 30.0;
  rootNode = pKernel->CreateComponent<udNode>();

  memset(&renderModels, 0, sizeof(renderModels));
  numRenderModels = 0;
}

udScene::~udScene()
{

}
