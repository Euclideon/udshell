
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
    EP_MAKE_PROPERTY_RO("rootNode", getRootNode, "Scene root node", nullptr, 0),
    EP_MAKE_PROPERTY_RO("bookmarkMap", getBookmarkMap, "Bookmark map", nullptr, 0),
  };
}
Array<const MethodInfo> Scene::getMethods() const
{
  return{
    EP_MAKE_METHOD(makeDirty, "Force a dirty signal"),
    EP_MAKE_METHOD(addBookmarkFromCamera, "Add a Bookmark from Camera"),
    EP_MAKE_METHOD(addBookmark, "Add a Bookmark"),
    EP_MAKE_METHOD(removeBookmark, "Remove a Bookmark"),
    EP_MAKE_METHOD(renameBookmark, "Rename a Bookmark"),
    EP_MAKE_METHOD_EXPLICIT("findBookmark", findBookmarkInternal, "Find a Bookmark"),
  };
}
Array<const EventInfo> Scene::getEvents() const
{
  return{
    EP_MAKE_EVENT(dirty, "Scene dirty event"),
  };
}

bool SceneImpl::InputEvent(const ep::InputEvent &ev)
{
  // do anything here?
  //...

  // pass input to the hierarchy...
  return rootNode->inputEvent(ev);
}

void SceneImpl::Update(double timeDelta)
{
  // do anything here?
  //...

  // update the hierarchy...
  if (rootNode->update(timeDelta))
    MakeDirty();
}

RenderableSceneRef SceneImpl::GetRenderScene()
{
  if (!bDirty)
    return spCache;

  RenderScene scene;
  rootNode->doRender(scene, rootNode->getMatrix());

  spCache = Convert(scene, getKernel()->getImpl()->GetRenderer().get());

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
      UDModelImpl *pImpl = in.spModel->getImpl<UDModelImpl>();
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
    MaterialImpl *pMatImpl = in.spMaterial->getImpl<MaterialImpl>();

    out.blendMode = pMatImpl->blendMode;
    out.cullMode = pMatImpl->cullMode;
    out.depthCompareFunc = pMatImpl->depthCompareFunc;

    out.primType = s_PrimTypeMap[in.renderList.type];
    out.numVertices = in.renderList.numVertices;
    out.firstIndex = in.renderList.firstIndex;
    out.firstVertex = in.renderList.firstVertex;

    out.spProgram = pMatImpl->spShaderProgram;
    out.setViewProjectionUniform = false;
    out.setViewRenderSize = false;
    out.setViewDisplaySize = false;

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

    if (in.spIndices)
      out.index = shared_pointer_cast<RenderArray>(pRenderer->GetRenderBuffer(in.spIndices, Renderer::RenderResourceType::IndexArray));

    Array<RenderShaderProperty> uniforms(Reserve, pMatImpl->uniforms.size());
    for (auto kvp : pMatImpl->uniforms)
    {
      if (kvp.value.current)
      {
        // TODO: Create mechanism for uniforms that are provided by the system. (projection, view, back buffer dimensions etc)
        if (kvp.key.eq("u_mfwvp"))
        {
          out.viewProjection = RenderShaderProperty{ kvp.value.data, kvp.value.uniformIndex };
          out.setViewProjectionUniform = true;
        }
        else if (kvp.key.eq("u_viewRenderSize"))
        {
          out.viewRenderSize = RenderShaderProperty{ kvp.value.data, kvp.value.uniformIndex };
          out.setViewRenderSize= true;
        }
        else if (kvp.key.eq("u_viewDisplaySize"))
        {
          out.viewDisplaySize = RenderShaderProperty{ kvp.value.data, kvp.value.uniformIndex };
          out.setViewDisplaySize = true;
        }
        else
          uniforms.pushBack(RenderShaderProperty{ kvp.value.data, kvp.value.uniformIndex } );
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
  rootNode = getKernel()->createComponent<Node>();

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
  epscope(fail) { if (!spSceneDS) getKernel()->logError("Failed to load scene file \"{0}\"", filePath); };
  spSceneDS = getKernel()->createComponent<GeomSource>({ { "src", filePath } });

  NodeRef spNode;
  if (spSceneDS->getNumResources() > 0)
  {
    for (NodeRef &child : rootNode->children())
      child->detach();

    spNode = spSceneDS->getResourceAs<Node>(0);

    rootNode->addChild(spNode);
    AddModelsToResourceManager();
    pInstance->getMetadata()->insert("url", filePath);
  }
}

void SceneImpl::AddModelsToResourceManager()
{
  Variant::VarMap modelMap;
  ResourceManagerRef spRM = getKernel()->getResourceManager();

  BuildModelMap(rootNode, modelMap);

  for (auto kvp : modelMap)
    spRM->addResource(kvp.value.as<UDModelRef>());
}

void SceneImpl::BuildModelMap(NodeRef spNode, Variant::VarMap &modelMap)
{
  for (NodeRef &spChild : spNode->children())
  {
    BuildModelMap(spChild, modelMap);

    UDNodeRef spUDNode;

    if (!spChild->isType<UDNode>())
      continue;

    spUDNode = component_cast<UDNode>(spChild);

    UDModelRef spUDModel = spUDNode->getUDModel();
    Variant filePath = spUDModel->getMetadata()->get("url");
    if (filePath.is(Variant::Type::String))
      modelMap.replace(filePath, spUDModel);
  }
}

void SceneImpl::AddBookmarkFromCamera(String bmName, CameraRef camera)
{
  if (!bmName || !camera)
    return;

  Double4x4 m = camera->getCameraMatrix();
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
  Variant::VarMap::MapType bookmarksSave;

  size_t index = 0;
  for (auto bm : bookmarks)
  {
    Variant::VarMap::MapType bmSave = Variant(bm.value).claimMap();
    bmSave.insert("name", bm.key);
    bookmarksSave.insert(MutableString<16>(Format, "bookmark{0}", index), std::move(bmSave));
    index++;
  }

  return std::move(bookmarksSave);
}

Variant SceneImpl::Save() const
{
  Variant::VarMap::MapType map;

  Variant url = pInstance->getMetadata()->get("url");
  if (url.is(Variant::Type::String))
  {
    String urlString = url.asString();
    if (!urlString.empty())
      map.insert("url", urlString);
  }

  if(!bookmarks.empty())
    map.insert("bookmarks", SaveBookmarks());

  return std::move(map);
}

}  // namespace ep
