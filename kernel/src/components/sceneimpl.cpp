
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

Array<const PropertyInfo> Scene::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("rootNode", GetRootNode, "Scene root node", nullptr, 0),
    EP_MAKE_PROPERTY_RO("bookmarkMap", GetBookmarkMap, "Bookmark map", nullptr, 0),
  };
}
Array<const MethodInfo> Scene::getMethods() const
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
Array<const EventInfo> Scene::getEvents() const
{
  return{
    EP_MAKE_EVENT(Dirty, "Scene dirty event"),
  };
}

bool SceneImpl::InputEvent(const ep::InputEvent &ev)
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
  rootNode->DoRender(scene, rootNode->GetMatrix());

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

  if (scene.ud.length)
  {
    // We must reserve the memory required before hand as the RenderContext contains internal pointers
    // that won't be fixed up when pushBack does a realloc.
    cache->ud.reserve(scene.ud.length);
    for (const auto &in : scene.ud)
    {
      UDJob &job = cache->ud.pushBack();
      job.spModel = in.spModel;
      UDModelImpl *pImpl = in.spModel->GetImpl<UDModelImpl>();
      pImpl->CopyRenderContext(&job.context);
      job.context.matrix = Mul(in.matrix, job.context.matrix);

      job.udNodePtr = in.udNodePtr;

      udRenderModel *pRenderModel = reinterpret_cast<udRenderModel*>(&job.context);
      const BufferRef &filterConstants = pImpl->constantBuffers[UDConstantDataType::VoxelFilter];
      if (filterConstants)
      {
        job.constantBuffers[UDConstantDataType::VoxelFilter] = shared_pointer_cast<RenderConstantBuffer>(pRenderer->GetConstantBuffer(filterConstants));
        pRenderModel->pFilterData = job.constantBuffers[UDConstantDataType::VoxelFilter]->pBuffer;
      }

      const BufferRef &voxelConstants = pImpl->constantBuffers[UDConstantDataType::VoxelShader];
      if (voxelConstants)
      {
        if (voxelConstants == filterConstants)
        {
          pRenderModel->pVoxelShaderData = pRenderModel->pFilterData;
        }
        else
        {
          job.constantBuffers[UDConstantDataType::VoxelShader] = shared_pointer_cast<RenderConstantBuffer>(pRenderer->GetConstantBuffer(voxelConstants));
          pRenderModel->pVoxelShaderData = job.constantBuffers[UDConstantDataType::VoxelShader]->pBuffer;
        }
      }

      const BufferRef &pixelConstants = pImpl->constantBuffers[UDConstantDataType::PixelShader];
      if (pixelConstants)
      {
        if (pixelConstants == voxelConstants)
        {
          pRenderModel->pPixelShaderData = pRenderModel->pVoxelShaderData;
        }
        else if (pixelConstants == filterConstants)
        {
          pRenderModel->pPixelShaderData = pRenderModel->pFilterData;
        }
        else
        {
          job.constantBuffers[UDConstantDataType::PixelShader] = shared_pointer_cast<RenderConstantBuffer>(pRenderer->GetConstantBuffer(pixelConstants));
          pRenderModel->pPixelShaderData = job.constantBuffers[UDConstantDataType::PixelShader]->pBuffer;
        }
      }
    }
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

    Array<RenderShaderProperty> uniforms(Reserve, pMatImpl->uniforms.size());
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
    {
      Array<RenderShaderProperty> outUniforms;

      for (size_t i = 0; i < uniforms.length; i++) // set up textures
      {
        if (uniforms[i].data.is(Variant::SharedPtrType::Component) && uniforms[i].data.asComponent()->isType<ArrayBuffer>()) // if the uniform is a texture
        {
          ArrayBufferRef spImage = uniforms[i].data.as<ArrayBufferRef>();
          RenderTextureRef spRenderTexture = shared_pointer_cast<RenderTexture>(pRenderer->GetRenderBuffer(spImage, Renderer::RenderResourceType::Texture)); // TODO: set texture parameters

          out.textures.pushBack(RenderableTexture{ uniforms[i].index, spRenderTexture });
        }
        else
          outUniforms.pushBack(uniforms[i]);
      }

      out.uniforms = std::move(outUniforms);
    }
  }

  return cache;
}

SceneImpl::SceneImpl(Component *_pInstance, Variant::VarMap initParams)
  : ImplSuper(_pInstance)
{
  timeStep = 1.0 / 30.0;
  rootNode = GetKernel()->CreateComponent<Node>();

  Variant *pSrc = initParams.get("url");
  if (pSrc && pSrc->is(Variant::Type::String))
    LoadSceneFile(pSrc->asString());

  Variant *pMap = initParams.get("bookmarks");
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

    if (!spChild->isType<UDNode>())
      continue;

    spUDNode = component_cast<UDNode>(spChild);

    UDModelRef spUDModel = spUDNode->GetUDModel();
    Variant filePath = spUDModel->GetMetadata()->Get("url");
    if (filePath.is(Variant::Type::String))
      modelMap.replace(filePath, spUDModel);
  }
}

void SceneImpl::AddBookmarkFromCamera(String bmName, CameraRef camera)
{
  if (!bmName || !camera)
    return;

  Double4x4 m = camera->GetCameraMatrix();
  Bookmark bm = { m.axis.t.toVector3(), m.extractYPR() };
  KVP<SharedString, Bookmark> kvp(bmName, bm);
  bookmarks.insert(std::move(kvp));
}

void SceneImpl::AddBookmark(String bmName, const Bookmark &bm)
{
  if (!bmName)
    return;
  bookmarks.replace(KVP<SharedString, Bookmark>(bmName, bm));
}

void SceneImpl::RemoveBookmark(String bmName)
{
  if (!bmName)
    return;
  bookmarks.remove(bmName);
}

void SceneImpl::RenameBookmark(String oldName, String newName)
{
  Bookmark *pBm = bookmarks.get(oldName);
  if (pBm)
  {
    bookmarks.insert(newName, *pBm);
    bookmarks.remove(oldName);
  }
}

void SceneImpl::LoadBookmarks(Variant::VarMap bm)
{
  for (auto kvp : bm)
  {
    if (kvp.value.is(Variant::SharedPtrType::AssocArray))
    {
      Variant::VarMap bmSaveMap = kvp.value.asAssocArray();

      Variant *pName = bmSaveMap.get("name");
      Variant *pPosition = bmSaveMap.get("position");
      Variant *pOrientation = bmSaveMap.get("orientation");
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
    bmSave.insert("name", bm.key);
    bookmarksSave.insert(MutableString<16>(Format, "bookmark{0}", index), std::move(bmSave));
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
      map.insert("url", urlString);
  }

  if(!bookmarks.empty())
    map.insert("bookmarks", SaveBookmarks());

  return map;
}

}  // namespace ep
