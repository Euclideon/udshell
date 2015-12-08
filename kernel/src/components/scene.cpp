
#include "scene.h"
#include "view.h"
#include "kernel.h"
#include "nodes/camera.h"
#include "renderscene.h"

namespace kernel {

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

Scene::Scene(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams) :
  Component(pType, pKernel, uid, initParams)
{
  timeStep = 1.0 / 30.0;
  rootNode = pKernel->CreateComponent<Node>();

  Variant *pMap = initParams.Get("Bookmarks");
  if (pMap)
  {
    Variant::VarMap bms = pMap->asAssocArray();
    for (auto node : bms)
      bookmarks.Insert(node.key.asSharedString(), node.value.as<Bookmark>());
  }

  memset(&renderModels, 0, sizeof(renderModels));
  numRenderModels = 0;
}

Scene::~Scene()
{

}

void Scene::AddBookMark(String bmName, CameraRef camera)
{
  if (!bmName || !camera)
    return;

  Double4x4 m = camera->GetCameraMatrix();
  Bookmark bm = { m.axis.t.toVector3(), m.extractYPR() };
  KVP<SharedString, Bookmark> kvp(bmName, bm);
  bookmarks.Insert(std::move(kvp));
}

void Scene::RemoveBookMark(String bmName)
{
  if (!bmName)
    return;

  bookmarks.Remove(bmName);
}

Variant Scene::Save() const
{
  Variant::VarMap map;
  map.Insert("Bookmarks", bookmarks);
  return map;
}

}  // namespace kernel
