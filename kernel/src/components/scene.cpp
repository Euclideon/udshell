
#include "scene.h"
#include "view.h"
#include "kernel.h"
#include "ep/cpp/component/node/camera.h"
#include "renderscene.h"

namespace ep {

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

  Variant *pMap = initParams.Get("bookmarks");
  if (pMap && pMap->is(Variant::Type::AssocArray))
    LoadBookMarks(pMap->asAssocArray());

  memset(&renderModels, 0, sizeof(renderModels));
  numRenderModels = 0;
}

Scene::~Scene()
{

}

void Scene::AddBookMarkFromCamera(String bmName, CameraRef camera)
{
  if (!bmName || !camera)
    return;

  Double4x4 m = camera->GetCameraMatrix();
  Bookmark bm = { m.axis.t.toVector3(), m.extractYPR() };
  KVP<SharedString, Bookmark> kvp(bmName, bm);
  bookmarks.Insert(std::move(kvp));
}

void Scene::AddBookMark(String bmName, const Bookmark &bm)
{
  if (!bmName)
    return;

  KVP<SharedString, Bookmark> kvp(bmName, bm);
  bookmarks.Insert(std::move(kvp));
}

void Scene::RemoveBookMark(String bmName)
{
  if (!bmName)
    return;

  bookmarks.Remove(bmName);
}

void Scene::RenameBookMark(String oldName, String newName)
{
  Bookmark *pBm = bookmarks.Get(oldName);
  if (pBm)
  {
    bookmarks.Remove(oldName);
    bookmarks.Insert(newName, *pBm);
  }
}

void Scene::LoadBookMarks(Variant::VarMap bm)
{
  for (auto kvp : bm)
  {
    if (kvp.value.is(Variant::Type::AssocArray))
    {
      Variant::VarMap bmSaveMap = kvp.value.asAssocArray();

      Variant *pName = bmSaveMap.Get("name");
      Variant *pPosition = bmSaveMap.Get("position");
      Variant *pOrientation = bmSaveMap.Get("orientation");
      if (!pOrientation || !pPosition || !pName)
        continue;

      auto posArray = pPosition->as<Array<double, 3>>();
      auto oriArray = pOrientation->as<Array<double, 3>>();
      if (posArray.length != 3 || oriArray.length != 3)
        continue;

      String bmName = pName->asString();
      Double3 bmPosition = Double3::create(posArray[0], posArray[1], posArray[2]);
      Double3 bmOrientation = Double3::create(oriArray[0], oriArray[1], oriArray[2]);

      AddBookMark(bmName, { bmPosition, bmOrientation });
    }
  }
}

Variant Scene::SaveBookMarks() const
{
  Variant::VarMap bookmarksSave;

  size_t index = 0;
  for (auto bm : bookmarks)
  {
    Variant bmVar = bm.value;
    Variant::VarMap bmSave = bmVar.asAssocArray();
    bmSave.Insert("name", bm.key);
    bookmarksSave.Insert(MutableString<16>(Format, "bookmark{0}", index), std::move(bmSave));
    index++;
  }

  return bookmarksSave;
}

Variant Scene::Save() const
{
  Variant::VarMap map;
  if(!bookmarks.Empty())
    map.Insert("bookmarks", SaveBookMarks());
  return map;
}

}  // namespace ep
