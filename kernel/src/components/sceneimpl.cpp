
#include "sceneimpl.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/node/camera.h"
#include "renderscene.h"
#include "components/resources/udmodelimpl.h"
#include "components/datasources/geomsource.h"
#include "ep/cpp/component/node/udnode.h"
#include "ep/cpp/component/resourcemanager.h"
#include "ep/cpp/component/resource/metadata.h"
#include "kernelimpl.h"
#include "renderscene.h"
#include "components/resources/arraybufferimpl.h"
#include "components/resources/materialimpl.h"

namespace ep {

Array<const PropertyInfo> Scene::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO(RootNode, "Scene root node", nullptr, 0),
    EP_MAKE_PROPERTY_RO(BookmarkMap, "Bookmark map", nullptr, 0),
  };
}
Array<const MethodInfo> Scene::GetMethods() const
{
  return{
    EP_MAKE_METHOD(MakeDirty, "Force a dirty signal"),
    EP_MAKE_METHOD(AddBookmarkFromCamera, "Add a Bookmark from Camera"),
    EP_MAKE_METHOD(AddBookmark, "Add a Bookmark"),
    EP_MAKE_METHOD(RemoveBookmark, "Remove a Bookmark"),
    EP_MAKE_METHOD(RenameBookmark, "Rename a Bookmark"),
    EP_MAKE_METHOD_EXPLICIT("FindBookmark", FindBookmark_Internal, "Find a Bookmark"),
  };
}
Array<const EventInfo> Scene::GetEvents() const
{
  return{
    EP_MAKE_EVENT(Dirty, "Scene dirty event"),
  };
}

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

  spCache = Convert(scene, GetKernel()->GetImpl()->GetRenderer().ptr());

  bDirty = false;
  return spCache;
}

static epPrimitiveType s_PrimTypeMap[] =
{
  epPT_Points,   // PrimType::Points
  epPT_Lines,    // PrimType::Lines
  epPT_Triangles // PrimType::Triagnles
};

RenderableSceneRef SceneImpl::Convert(RenderScene &scene, Renderer *pRenderer)
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

    if (iS.voxelVarDelegate)
    {
      oS.pVoxelShader = UDRenderableState::VoxelVarDelegateShaderFunc;
      oS.voxelVarDelegate = iS.voxelVarDelegate;
    }

    oS.flags = (udRenderFlags)iS.flags;
    oS.startingRoot = iS.startingRoot;
  }

  for (const auto &in : scene.geom)
  {
    auto &out = cache->geom.pushBack();
    out.matrix = in.matrix;
    MaterialImpl *pMatImpl = in.spMaterial->GetImpl<MaterialImpl>();

    out.stencilStates = pMatImpl->stencilStates;
    out.blendMode = pMatImpl->blendMode;
    out.cullMode = pMatImpl->cullMode;
    out.depthCompareFunc = pMatImpl->depthCompareFunc;

    out.primType = s_PrimTypeMap[in.renderList.type];
    out.numVertices = in.renderList.numVertices;
    out.firstIndex = in.renderList.firstIndex;
    out.firstVertex = in.renderList.firstVertex;

    out.spProgram = pMatImpl->spShaderProgram;
    out.setViewProjectionUniform = false;

    if (in.spShaderInputConfig)
      out.spShaderInputConfig = shared_pointer_cast<RenderShaderInputConfig>(in.spShaderInputConfig);
    else
      out.spShaderInputConfig = pRenderer->GetShaderInputConfig(in.vertexArrays, pMatImpl->spShaderProgram, in.retainShaderInputConfig);

    out.arrays.resize(in.vertexArrays.length);

    for (const auto a : out.arrays)
      out.epArrays.pushBack(nullptr);

    for (int stream : out.spShaderInputConfig->GetActiveStreams())
    {
      out.arrays[stream] = shared_pointer_cast<RenderArray>(pRenderer->GetRenderBuffer(in.vertexArrays[stream], Renderer::RenderResourceType::VertexArray));
      out.epArrays[stream] = out.arrays[stream]->pArray;
    }

    out.index = shared_pointer_cast<RenderArray>(pRenderer->GetRenderBuffer(in.spIndices, Renderer::RenderResourceType::IndexArray));

    Array<RenderShaderProperty> uniforms(Reserve, pMatImpl->uniforms.Size());
    for (auto kvp : pMatImpl->uniforms)
    {
      if (kvp.value.current)
      {
        // TODO: Create mechanism for uniforms that are provided by the system. (projection, view, back buffer dimensions etc)
        if (kvp.key.eq("u_mfwvp"))
        {
          out.viewProjection = RenderShaderProperty{ kvp.value.data, (int)kvp.value.element.location };
          out.setViewProjectionUniform = true;
        }
        else
          uniforms.pushBack(RenderShaderProperty{ kvp.value.data, (int)kvp.value.element.location } );
      }
    }

    if (uniforms.length)
      out.uniforms = uniforms;

    // TODO: texures
  }

  return cache;
}

SceneImpl::SceneImpl(Component *_pInstance, Variant::VarMap initParams)
  : ImplSuper(_pInstance)
{
  timeStep = 1.0 / 30.0;
  rootNode = GetKernel()->CreateComponent<Node>();

  Variant *pSrc = initParams.Get("url");
  if (pSrc && pSrc->is(Variant::Type::String))
    LoadSceneFile(pSrc->asString());

  Variant *pMap = initParams.Get("bookmarks");
  if (pMap && pMap->is(Variant::SharedPtrType::AssocArray))
    LoadBookmarks(pMap->asAssocArray());

  memset(&renderModels, 0, sizeof(renderModels));
  numRenderModels = 0;
}

void SceneImpl::LoadSceneFile(String filePath)
{
  GeomSourceRef spSceneDS;
  epscope(fail) { if (!spSceneDS) GetKernel()->LogError("Failed to load scene file \"{0}\"", filePath); };
  spSceneDS = GetKernel()->CreateComponent<GeomSource>({ { "src", filePath } });

  NodeRef spNode;
  if (spSceneDS->GetNumResources() > 0)
  {
    for (NodeRef &child : rootNode->Children())
      child->Detach();

    spNode = spSceneDS->GetResourceAs<Node>(0);

    rootNode->AddChild(spNode);
    AddModelsToResourceManager();
    pInstance->GetMetadata()->Insert("url", filePath);
  }
}

void SceneImpl::AddModelsToResourceManager()
{
  Variant::VarMap modelMap;
  ResourceManagerRef spRM = GetKernel()->GetResourceManager();

  BuildModelMap(rootNode, modelMap);

  for (auto kvp : modelMap)
    spRM->AddResource(kvp.value.as<UDModelRef>());
}

void SceneImpl::BuildModelMap(NodeRef spNode, Variant::VarMap &modelMap)
{
  for (NodeRef &spChild : spNode->Children())
  {
    BuildModelMap(spChild, modelMap);

    UDNodeRef spUDNode;

    if (!spChild->IsType<UDNode>())
      continue;

    spUDNode = component_cast<UDNode>(spChild);

    UDModelRef spUDModel = spUDNode->GetUDModel();
    Variant filePath = spUDModel->GetMetadata()->Call("get", "url");
    if (filePath.is(Variant::Type::String))
      modelMap.Insert(filePath, spUDModel);
  }
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

  Variant url = pInstance->GetMetadata()->Get("url");
  if (url.is(Variant::Type::String))
  {
    String urlString = url.asString();
    if (!urlString.empty())
      map.Insert("url", urlString);
  }

  if(!bookmarks.Empty())
    map.Insert("bookmarks", SaveBookmarks());

  return map;
}

}  // namespace ep
