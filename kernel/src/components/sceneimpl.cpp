
#include "sceneimpl.h"
#include "kernel.h"
#include "ep/cpp/component/node/camera.h"
#include "renderscene.h"
#include "components/resources/udmodelimpl.h"

namespace ep {

bool SceneImpl::InputEvent(const epInputEvent &ev)
{
  // do anything here?
  //...

  // pass input to the hierarchy...
  return rootNode->InputEvent(ev);
}

void SceneImpl::Update(double timeDelta)
{
  // do anything here?
  //...

  // update the hierarchy...
  if (rootNode->Update(timeDelta))
    MakeDirty();
}

RenderableSceneRef SceneImpl::GetRenderScene()
{
  if (!bDirty)
    return spCache;

  RenderScene scene;
  rootNode->Render(scene, rootNode->GetMatrix());

  spCache = Convert(scene);

  bDirty = false;
  return spCache;
}

RenderableSceneRef SceneImpl::Convert(RenderScene &scene)
{
  RenderableSceneRef cache = RenderableSceneRef::create();

  for (const auto &in : scene.ud)
  {
    auto &out = cache->ud.pushBack();
    out.spModel = in.spModel;

    const UDRenderState &iS = in.renderState;
    UDRenderableState &oS = out.renderState;
    memset(&oS, 0, sizeof(udRenderModel));

    oS.pOctree = in.spModel->GetImpl<UDModelImpl>()->pOctree;

    if (iS.useClip)
    {
      oS.clipArea = udRenderClipArea { iS.rect.x, iS.rect.y, iS.rect.width, iS.rect.height };
      oS.pClip = &oS.clipArea;
    }

    oS.matrix = iS.matrix;
    oS.pWorldMatrixD = &oS.matrix.a[0];

    if (iS.simpleVoxelDel.GetMemento())
    {
      oS.pVoxelShader = UDRenderableState::VoxelShaderFunc;
      oS.simpleVoxelDel = iS.simpleVoxelDel;
    }

    oS.flags = (udRenderFlags)iS.flags;
    oS.startingRoot = iS.startingRoot;
  }

  for (const auto &in : scene.geom)
  {
    auto &out = cache->geom.pushBack();

/* in...
    Double4x4 matrix;

    SharedArray<VertexArray> vertexArrays;
    ArrayBufferRef spIndices;
    MaterialRef spMaterial;

    Array<RenderList, 1> renderList;

    Delegate<void(SharedPtr<RefCounted>)> programCacheCallback;
    Delegate<void(SharedPtr<RefCounted>)> vertexFormatCacheCallback;


    // out...
    Double4x4 matrix;

    uint32_t numTextures, numArrays;
    RenderTextureRef textures[8];
    RenderArrayRef arrays[16];
    RenderArrayRef index;

    RenderShaderProgramRef spProgram;
    RenderVertexFormatRef spVertexFormat;

    BlendMode blendMode;
    CullMode cullMode;
*/

    out.matrix = in.matrix;

    out.arrays.resize(in.vertexArrays.length);
    for (size_t i = 0; i < in.vertexArrays.length; ++i)
      out.arrays.ptr[i] = in.vertexArrays.ptr[i].spArray;
/*
    out.textures.resize(in.vertexArrays.length);
    for (size_t i = 0; i < invertexArrays.length; ++i)
      out.textures.ptr[i] = in.vertexArrays.ptr[i].spArray;

    out.numTextures = in.numTextures;
    out.numArrays = in.numArrays;
*/
  }

  return cache;
}

SceneImpl::SceneImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  timeStep = 1.0 / 30.0;
  rootNode = GetKernel()->CreateComponent<Node>();

  Variant *pMap = initParams.Get("bookmarks");
  if (pMap && pMap->is(Variant::SharedPtrType::AssocArray))
    LoadBookmarks(pMap->asAssocArray());

  memset(&renderModels, 0, sizeof(renderModels));
  numRenderModels = 0;
}

void SceneImpl::AddBookmarkFromCamera(String bmName, CameraRef camera)
{
  if (!bmName || !camera)
    return;

  Double4x4 m = camera->GetCameraMatrix();
  Bookmark bm = { m.axis.t.toVector3(), m.extractYPR() };
  KVP<SharedString, Bookmark> kvp(bmName, bm);
  bookmarks.Insert(std::move(kvp));
}

void SceneImpl::AddBookmark(String bmName, const Bookmark &bm)
{
  if (!bmName)
    return;

  KVP<SharedString, Bookmark> kvp(bmName, bm);
  bookmarks.Insert(std::move(kvp));
}

void SceneImpl::RemoveBookmark(String bmName)
{
  if (!bmName)
    return;

  bookmarks.Remove(bmName);
}

void SceneImpl::RenameBookmark(String oldName, String newName)
{
  Bookmark *pBm = bookmarks.Get(oldName);
  if (pBm)
  {
    bookmarks.Insert(newName, *pBm);
    bookmarks.Remove(oldName);
  }
}

void SceneImpl::LoadBookmarks(Variant::VarMap bm)
{
  for (auto kvp : bm)
  {
    if (kvp.value.is(Variant::SharedPtrType::AssocArray))
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

      AddBookmark(bmName, { bmPosition, bmOrientation });
    }
  }
}

Variant SceneImpl::SaveBookmarks() const
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

Variant SceneImpl::Save() const
{
  Variant::VarMap map;
  if(!bookmarks.Empty())
    map.Insert("bookmarks", SaveBookmarks());
  return map;
}

}  // namespace ep
