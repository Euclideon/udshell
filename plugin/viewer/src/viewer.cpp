#include "viewer.h"

#include "ep/cpp/component/view.h"
#include "ep/cpp/component/scene.h"
#include "ep/cpp/component/node/simplecamera.h"
#include "ep/cpp/component/node/udnode.h"
#include "ep/cpp/component/resource/udmodel.h"
#include "ep/cpp/component/resource/metadata.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/component/resourcemanager.h"
#include "ep/cpp/component/commandmanager.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/resource/shader.h"
#include "ep/cpp/component/resource/material.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/node/geomnode.h"
#include "ep/cpp/component/primitivegenerator.h"

namespace ep {

Array<const PropertyInfo> Viewer::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO(SimpleCamera, "The Viewer's SimpleCamera Component", nullptr, 0),
    EP_MAKE_PROPERTY_RO(View, "The Viewer's View Component", nullptr, 0),
  };
}

Viewer::Viewer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Activity(pType, pKernel, uid, initParams)
{
  ResourceManagerRef spResourceManager = pKernel->GetResourceManager();

  spView = pKernel->CreateComponent<View>();
  spView->SetInputEventHook(Delegate<bool(ep::InputEvent)>(this, &Viewer::InputHook));

  Variant::VarMap sceneParams;
  const Variant *pSceneParams = initParams.get("scene");
  if (pSceneParams && pSceneParams->is(Variant::SharedPtrType::AssocArray))
    sceneParams = pSceneParams->asAssocArray();
  spScene = pKernel->CreateComponent<Scene>(sceneParams);

  Variant::VarMap cameraParams;
  const Variant *pCam = initParams.get("camera");
  if (pCam && pCam->is(Variant::SharedPtrType::AssocArray))
    cameraParams = pCam->asAssocArray();
  spCamera = pKernel->CreateComponent<SimpleCamera>(Variant::VarMap(cameraParams));

  const Variant *model = initParams.get("model");
  if (model)
  {
    if (model->is(Variant::Type::String))
    {
      // TODO: enable streamer once we have a tick running to update the streamer
      String modelSrc = model->asString();

      DataSourceRef spModelDS;
      epscope(fail) { if (!spModelDS) pKernel->LogError("Viewer -- Failed to load model\n"); };
      spModelDS = spResourceManager->LoadResourcesFromFile({ { "src", modelSrc }, { "useStreamer", true } });
      if (spModelDS->GetNumResources() > 0)
      {
        epscope(fail) { if (!spModel) pKernel->LogError("Viewer -- Failed to load model. Not a UDModel\n"); };
        spModel = spModelDS->GetResourceAs<UDModel>(0);
      }
    }
    else if (model->is(Variant::SharedPtrType::Component))
      spModel = model->as<UDModelRef>();
  }

  if (spModel)
  {
    UDNodeRef spUDNode = pKernel->CreateComponent<UDNode>();
    spUDNode->SetUDModel(spModel);
    spScene->GetRootNode()->AddChild(spUDNode);
    spScene->MakeDirty();
    spView->SetEnablePicking(true);
    spScene->AddBookmark(MutableString128(Format, "{0}_bookmark", Viewer::GetFileNameFromPath(model->asString())), { spModel->GetUDMatrix().axis.t.toVector3(), { 0, 0, 0 }});
  }

  spView->SetUDRenderFlags(UDRenderFlags::ClearTargets);

  spView->SetScene(spScene);
  spView->SetCamera(spCamera);

  UIComponentRef spViewport;
  epscope(fail) { if (!spViewport) pKernel->LogError("Error creating Viewport Component\n"); };
  spViewport = component_cast<UIComponent>(pKernel->CreateComponent("ui.viewport", Variant::VarMap{ { "view", spView } }));

  UIComponentRef spViewerUI;
  epscope(fail) { if(!spViewerUI) pKernel->LogError("Error creating Viewer UI Component\n"); };
  spViewerUI = component_cast<UIComponent>(pKernel->CreateComponent("viewer.mainui"));
  spViewerUI->Set("viewport", spViewport);
  spViewerUI->Subscribe("resourcedropped", Delegate<void(String, int, int)>(this, &Viewer::OnResourceDropped));

  epscope(fail) { if(!spUIBookmarks) pKernel->LogError("Error creating bookmarks UI Component\n"); };
  spUIBookmarks = component_cast<UIComponent>(pKernel->CreateComponent("ui.bookmarksui"));
  spUIBookmarks->Set("view", spView);
  spViewerUI->Set("bookmarkscomp", spUIBookmarks);

  // TODO: Bug EP-66
  /*UIComponentRef spUIResources;
  ComponentRef spComp = pKernel->FindComponent("resourcespanel");
  if (spComp)
    spUIResources = component_cast<UIComponent>(spComp);
  else
  {
    epscope(fail) { if (!spUIResources) pKernel->LogError("Error creating Resource Panel UI Component\n"); };
    spUIResources = component_cast<UIComponent>(pKernel->CreateComponent("ui.resourcespanel", Variant::VarMap{ { "name", "resourcespanel" } }));
  }
  spViewerUI->Set("resourcespanel", spUIResources);*/

  SetUI(spViewerUI);

  // Add bookmarks to UI
  auto bmMap = spScene->GetBookmarkMap();
  for (auto bm : bmMap)
    spUIBookmarks->Call("createbookmark", bm.key);

#if EP_DEBUG
  try
  {
    CreatePlatformLogo();
  }
  catch (EPException ex)
  {
    pKernel->LogWarning(0, "Could not load platform logo");
  }
#endif // EP_DEBUG
}

MutableString<260> Viewer::GetFileNameFromPath(String path) // TODO Move this to File after implising File
{
  String fName = path.getRightAtLast("/", false);
  if (fName.empty())
  {
    fName = path.getRightAtLast("\\", false);

    if (fName.empty())
      return path;
  }

  return fName;
}

void Viewer::OnResourceDropped(String resourceUID, int x, int y)
{
  ResourceManagerRef spResourceManager = pKernel->GetResourceManager();
  UDModelRef spUDModel;

  try
  {
    spUDModel = spResourceManager->GetResourceAs<UDModel>(resourceUID);
  }
  catch(EPException &)
  {
    LogDebug(2, "Can't drop \"{0}\" into viewport -- Component type is not supported", resourceUID);
    return;
  }

  UDNodeRef spUDNode = pKernel->CreateComponent<UDNode>();
  spUDNode->SetUDModel(spUDModel);

  AddSceneNodeAtViewPosition(spUDNode, x, y);
}

void Viewer::AddSceneNodeAtViewPosition(UDNodeRef spUDNode, int x, int y)
{
  const Double4x4 &cameraMatrix = spCamera->GetMatrix();

  BoundingVolume vol = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } };

  MetadataRef udMeta = spUDNode->GetUDModel()->GetMetadata();
  Variant header = udMeta->Get("octreeHeader");
  if (header.isValid())
    vol = header["boundingVolume"].as<BoundingVolume>();

  Double3 modelCenter = (vol.max - vol.min) / 2;

  double dist = 2.0;

  // Map mouse coordinates to a point on the plane at y=dist
  Dimensions<int> displayDims = spView->GetDimensions();

  double distX, distZ;

  if (spCamera->IsOrtho())
  {
    distX = spView->GetAspectRatio() * spCamera->GetOrthoHeight() / 2;
    distZ = spCamera->GetOrthoHeight() / 2;
  }
  else
  {
    // Perspective projection
    distX = dist * spView->GetAspectRatio() * Tan(spCamera->GetFovY() / 2);
    distZ = dist * Tan(spCamera->GetFovY() / 2);
  }

  double mouseCameraX = x * 2 * distX / (displayDims.width - 1) - distX;
  double mouseCameraZ = - ( y * 2 * distZ / (displayDims.height - 1) - distZ );

  // Set node's position relative to the camera, so that the model's center is positioned at the current mouse position.
  Double4 nodeCameraPos = Double4::create(mouseCameraX - modelCenter.x, dist, mouseCameraZ - modelCenter.z, 1.0);

  // Transform node's position from camera to world space
  Double3 cameraYPR = cameraMatrix.extractYPR();
  Double3 cameraTranslation = cameraMatrix.axis.t.toVector3();

  Double4x4 transformR = Double4x4::rotationYPR(cameraYPR);
  Double4x4 transformT = Double4x4::translation(cameraTranslation);

  Double4 nodePos = transformT * transformR * nodeCameraPos;

  // Set node's orientation so it faces towards the camera
  cameraYPR.y = -cameraYPR.y;
  Double4x4 nodeMatrix = Double4x4::rotationYPR(cameraYPR, nodePos.toVector3());

  spUDNode->SetMatrix(nodeMatrix);

  spScene->GetRootNode()->AddChild(spUDNode);
  spScene->MakeDirty();
}

void Viewer::StaticInit(ep::Kernel *pKernel)
{
  auto spCommandManager = pKernel->GetCommandManager();

  spCommandManager->RegisterCommand("togglebookmarkspanel", Delegate<void(Variant::VarMap)>(&Viewer::StaticToggleBookmarksPanel), "", ComponentID(), "Ctrl+Shift+B");
  // TODO: Bug EP-66
  //spCommandManager->RegisterCommand("toggleresourcespanel", Delegate<void(Variant::VarMap)>(&Viewer::StaticToggleResourcesPanel), "", ComponentID(), "Ctrl+Shift+R");
  spCommandManager->RegisterCommand("createbookmark", Delegate<void(Variant::VarMap)>(&Viewer::StaticCreateBookmark), "", ComponentID(), "Ctrl+B");
}

void Viewer::StaticToggleBookmarksPanel(Variant::VarMap params)
{
  Variant *pActivityVar = params.get("activity");
  ViewerRef spViewer = pActivityVar->as<ViewerRef>();

  spViewer->ToggleBookmarksPanel();
}

void Viewer::ToggleBookmarksPanel()
{
  GetUI()->Call("togglebookmarkspanel");
}

// TODO: Bug EP-66
/*void Viewer::StaticToggleResourcesPanel(Variant::VarMap params)
{
  Variant *pActivityVar = params.get("activity");
  ViewerRef spViewer = pActivityVar->as<ViewerRef>();

  spViewer->ToggleResourcesPanel();
}

void Viewer::ToggleResourcesPanel()
{
  GetUI()->Call("toggleresourcespanel");
}*/

void Viewer::StaticCreateBookmark(Variant::VarMap params)
{
  Variant *pActivityVar = params.get("activity");
  ViewerRef spViewer = pActivityVar->as<ViewerRef>();
  spViewer->CreateBookmark();
}

void Viewer::CreateBookmark()
{
  // TODO: Here we have to update the bookmark list separately for the front-end UI and the internal bookmarks list.
  // It would be nice if the QML could automatically update its bookmarks list from the internal bookmarks
  // Not sure how to do this currently
  Variant bookmarkName = spUIBookmarks->Call("createbookmark", "");
  spScene->AddBookmarkFromCamera(bookmarkName.asString(), spCamera);
}

void Viewer::Activate()
{
  GetKernel().UpdatePulse.Subscribe(Delegate<void(double)>(this, &Viewer::Update));
}

void Viewer::Deactivate()
{
  GetKernel().UpdatePulse.Unsubscribe(Delegate<void(double)>(this, &Viewer::Update));
}

void Viewer::Update(double timeStep)
{
  if (spScene)
    spScene->Update(timeStep);
}

Variant Viewer::Save() const
{
  Variant::VarMap params;

  if (spCamera)
    params.insert("camera", spCamera->Save());

  if (spModel)
  {
    Variant src = spModel->GetMetadata()->Get("url");
    if (src.is(Variant::Type::String))
    {
      String srcString = src.asString();

      if (!srcString.empty())
        params.insert("model", srcString);
    }
  }

  if (spScene)
    params.insert("scene", spScene->Save());

  return Variant(std::move(params));
}

void Viewer::CreatePlatformLogo()
{
  DataSourceRef spImageSource = pKernel->CreateDataSourceFromFile("libep/doc/images/platform_logo.png");
  ArrayBufferRef spImage = spImageSource->GetResourceAs<ArrayBuffer>("image0");

  // Vertex Shader
  ShaderRef vertexShader = pKernel->CreateComponent<Shader>();
  {
    vertexShader->SetType(ShaderType::VertexShader);

    const char shaderText[] = "attribute vec3 a_position;\n"
      "uniform mat4 u_mfwvp;\n"
      "void main()\n"
      "{\n"
      "  gl_Position = u_mfwvp == mat4(1.0) ? vec4(a_position, 1.0) : vec4(a_position, 1.0);\n"
      "  gl_TexCoord[0].xy = a_position.xy*0.5+0.5;\n"
      "}\n";
    vertexShader->SetCode(shaderText);
  }

  // Pixel Shader
  ShaderRef pixelShader = pKernel->CreateComponent<Shader>();
  {
    pixelShader->SetType(ShaderType::PixelShader);

    const char shaderText[] =
      "uniform sampler2D tex;"
      "void main()\n"
      "{\n"
      "  vec2 texCoord = gl_TexCoord[0].xy;\n"
      "  texCoord -= .8;\n"
      "  texCoord *= 5;\n"
      "  if (texCoord.x < 0 || texCoord.y < 0) discard;\n"
//       "  texCoord.y = 1 - texCoord.y;\n" // hack to flip the y axis
      "  vec4 col = texture(tex, texCoord);\n"
//       "  if (col.a != 1) discard;" // hack to stop flickering
//       "  gl_FragColor = vec4(col.b, col.g, col.r, col.a);\n" // hack to swap r and b
      "  gl_FragColor = col;\n"
      "}\n";
    pixelShader->SetCode(shaderText);
  }

  MaterialRef spMaterial = pKernel->CreateComponent<Material>();
  spMaterial->SetShader(ShaderType::VertexShader, vertexShader);
  spMaterial->SetShader(ShaderType::PixelShader, pixelShader);

  ArrayBufferRef spVertexBuffer = pKernel->CreateComponent<ArrayBuffer>();
  ArrayBufferRef spIndexBuffer = pKernel->CreateComponent<ArrayBuffer>();
  PrimitiveGeneratorRef generator = pKernel->CreateComponent<PrimitiveGenerator>();

  generator->GenerateQuad(spVertexBuffer, spIndexBuffer);

  MetadataRef spMetadata = spVertexBuffer->GetMetadata();
  spMetadata->Get("attributeinfo")[0].insertItem("name", "a_position");

  ModelRef spImageModel = pKernel->CreateComponent<Model>();

  spImageModel->AddVertexArray(spVertexBuffer);
  spImageModel->SetIndexArray(spIndexBuffer);
  spImageModel->SetMaterial(spMaterial);
  spImageModel->SetRenderList(RenderList{ PrimType::Triangles, size_t(0), size_t(0), size_t(6) });

  GeomNodeRef spGeomNode = pKernel->CreateComponent<GeomNode>();
  spGeomNode->SetModel(spImageModel);
  spScene->GetRootNode()->AddChild(spGeomNode);
  spImageNode = spGeomNode;

  spMaterial->SetMaterialProperty("tex", spImage);
}

bool Viewer::InputHook(ep::InputEvent ev)
{
  if (ev.eventType == ep::InputEvent::EventType::Key && ev.deviceType == ep::InputDevice::Keyboard) // if a key event happened on the keyboard
  {
    if (ev.key.state == 1 && ev.key.key == (int)ep::KeyCode::I) // if pressing down on the 'i' key then switch the logos visibility
    {
      NodeRef spRootNode = spScene->GetRootNode();
      bool contains = spRootNode->Children().exists(spImageNode);

      if (contains)
        spRootNode->RemoveChild(spImageNode);
      else
        spRootNode->AddChild(spImageNode);

      spScene->MakeDirty();
      return true; // we return true here so I is not used as input for another action
    }
  }

  return false; // return false so that this can be handled by another action
}

extern "C" bool epPluginAttach()
{
  Kernel::GetInstance()->RegisterComponentType<Viewer>();

#if 0
  // NOTE: Use to test off disk
  Kernel::GetInstance()->Call("registerqmlcomponents", "plugin/viewer/qml");
#else
  Kernel::GetInstance()->Call("registerqmlcomponents", ":/viewer");
#endif

  return true;
}

} // namespace ep
