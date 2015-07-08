
#include "udScene.h"
#include "udView.h"


static const udPropertyDesc props[] =
{
  {
    "model", // id
    "Model", // displayName
    "Model to render", // description
    udPropertyType::Integer, // type
    0, // arrayLength
    udPF_NoRead, // flags
    udPropertyDisplayType::Default, // displayType
    nullptr,
    nullptr
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
  [](){ return udR_Success; },             // pInitRender
  udScene::Create, // pCreateInstance

  udSlice<const udPropertyDesc>(props, ARRAY_LENGTH(props)) // propeties
};


udResult udScene::InputEvent(const udInputEvent &ev)
{
  // do anything here?
  //...

  // pass input to the hierarchy...
  if (pRootNode)
    return pRootNode->InputEvent(ev);
  return udR_EventNotHandled;
}

udResult udScene::Update(double timeDelta)
{
  // do anything here?
  //...

  // update the hierarchy...
  if (pRootNode)
    return pRootNode->Update(timeDelta);
  return udR_Success;
}

udResult udScene::Render(udViewRef pView)
{
  // TODO: this will go away, and be represented by

  udRenderView *pRenderView = pView->GetUDRenderView();

  udRenderModel *pRenderModels[16];
  for (size_t i = 0; i < numRenderModels; ++i)
    pRenderModels[i] = &renderModels[i];

  udRender_Render(pRenderView, pRenderModels, (int)numRenderModels, &options);

  return udR_Success;
}

udResult udScene::SetRenderModels(struct udRenderModel models[], size_t numModels)
{
  for (size_t i = 0; i < numModels; ++i)
    renderModels[i] = models[i];
  numRenderModels = numModels;

  return udR_Success;
}

udResult udScene::SetRenderOptions(const struct udRenderOptions &options)
{
  this->options = options;

  return udR_Success;
}

udRenderScene *udScene::GetRenderScene()
{
  if (!bDirty && pRenderScene)
    return pRenderScene;

  // begin a new hierarchy...
  pRenderScene = nullptr; // TODO: create a new empty one...

  // call into the hierarchy to populate the render scene with renderables...
  if (pRootNode)
    pRootNode->Render(udDouble4x4::identity());

  return pRenderScene;
}

udScene::udScene(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams) :
  udComponent(pType, pKernel, uid, initParams)
{
  timeStep = 1.0 / 30.0;
  pRootNode = nullptr;
  bDirty = false;

  pRenderScene = nullptr;

  memset(&options, 0, sizeof(options));
  memset(&renderModels, 0, sizeof(renderModels));
  numRenderModels = 0;
}

udScene::~udScene()
{

}
