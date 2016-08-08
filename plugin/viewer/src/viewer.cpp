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

Array<const PropertyInfo> Viewer::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("simpleCamera", getSimpleCamera, "The Viewer's SimpleCamera Component", nullptr, 0),
    EP_MAKE_PROPERTY_RO("view", getView, "The Viewer's View Component", nullptr, 0),
  };
}

Viewer::Viewer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Activity(pType, pKernel, uid, initParams)
{
  ResourceManagerRef spResourceManager = pKernel->getResourceManager();

  spView = pKernel->createComponent<View>();
  spView->setInputEventHook(Delegate<bool(ep::InputEvent)>(this, &Viewer::inputHook));

  Variant::VarMap sceneParams;
  const Variant *pSceneParams = initParams.get("scene");
  if (pSceneParams && pSceneParams->is(Variant::SharedPtrType::AssocArray))
    sceneParams = pSceneParams->asAssocArray();
  spScene = pKernel->createComponent<Scene>(sceneParams);

  Variant::VarMap cameraParams;
  const Variant *pCam = initParams.get("camera");
  if (pCam && pCam->is(Variant::SharedPtrType::AssocArray))
    cameraParams = pCam->asAssocArray();
  spCamera = pKernel->createComponent<SimpleCamera>(cameraParams);

  const Variant *model = initParams.get("model");
  if (model)
  {
    if (model->is(Variant::Type::String))
    {
      // TODO: enable streamer once we have a tick running to update the streamer
      String modelSrc = model->asString();

      DataSourceRef spModelDS;
      epscope(fail) { if (!spModelDS) pKernel->logError("Viewer -- Failed to load model\n"); };
      spModelDS = spResourceManager->loadResourcesFromFile({ { "src", modelSrc }, { "useStreamer", true } });
      if (spModelDS->getNumResources() > 0)
      {
        epscope(fail) { if (!spModel) pKernel->logError("Viewer -- Failed to load model. Not a UDModel\n"); };
        spModel = spModelDS->getResourceAs<UDModel>(0);
      }
    }
    else if (model->is(Variant::SharedPtrType::Component))
      spModel = model->as<UDModelRef>();
  }

  if (spModel)
  {
    UDNodeRef spUDNode = pKernel->createComponent<UDNode>();
    spUDNode->setUDModel(spModel);
    spScene->getRootNode()->addChild(spUDNode);
    spScene->makeDirty();
    spView->setEnablePicking(true);
    spScene->addBookmark(MutableString128(Format, "{0}_bookmark", Viewer::getFileNameFromPath(model->asString())), { spModel->getUDMatrix().axis.t.toVector3(), { 0, 0, 0 }});
  }

  spView->setUDRenderFlags(UDRenderFlags::ClearTargets);

  spView->setScene(spScene);
  spView->setCamera(spCamera);

  UIComponentRef spViewport;
  epscope(fail) { if (!spViewport) pKernel->logError("Error creating Viewport Component\n"); };
  spViewport = component_cast<UIComponent>(pKernel->createComponent("ui.Viewport", Variant::VarMap{ { "view", spView } }));

  UIComponentRef spViewerUI;
  epscope(fail) { if(!spViewerUI) pKernel->logError("Error creating Viewer UI Component\n"); };
  spViewerUI = component_cast<UIComponent>(pKernel->createComponent("viewer.MainUI"));
  spViewerUI->set("viewport", spViewport);
  spViewerUI->subscribe("resourcedropped", Delegate<void(String, int, int)>(this, &Viewer::onResourceDropped));

  epscope(fail) { if(!spUIBookmarks) pKernel->logError("Error creating bookmarks UI Component\n"); };
  spUIBookmarks = component_cast<UIComponent>(pKernel->createComponent("ui.BookmarksUI"));
  spUIBookmarks->set("view", spView);
  spViewerUI->set("bookmarkscomp", spUIBookmarks);

  // TODO: Bug EP-66
  /*UIComponentRef spUIResources;
  ComponentRef spComp = pKernel->FindComponent("resourcespanel");
  if (spComp)
    spUIResources = component_cast<UIComponent>(spComp);
  else
  {
    epscope(fail) { if (!spUIResources) pKernel->logError("Error creating Resource Panel UI Component\n"); };
    spUIResources = component_cast<UIComponent>(pKernel->CreateComponent("ui.resourcespanel", Variant::VarMap{ { "name", "resourcespanel" } }));
  }
  spViewerUI->Set("resourcespanel", spUIResources);*/

  setUI(spViewerUI);

  // Add bookmarks to UI
  auto bmMap = spScene->getBookmarkMap();
  for (auto bm : bmMap)
    spUIBookmarks->call("createbookmark", bm.key);

#if EP_DEBUG
  try
  {
    createPlatformLogo();
  }
  catch (EPException ex)
  {
    pKernel->logWarning(0, "Could not load platform logo");
  }
#endif // EP_DEBUG
}

MutableString<260> Viewer::getFileNameFromPath(String path) // TODO Move this to File after implising File
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

void Viewer::onResourceDropped(String resourceUID, int x, int y)
{
  ResourceManagerRef spResourceManager = pKernel->getResourceManager();
  UDModelRef spUDModel;

  try
  {
    spUDModel = spResourceManager->getResourceAs<UDModel>(resourceUID);
  }
  catch(EPException &)
  {
    logDebug(2, "Can't drop \"{0}\" into viewport -- Component type is not supported", resourceUID);
    return;
  }

  UDNodeRef spUDNode = pKernel->createComponent<UDNode>();
  spUDNode->setUDModel(spUDModel);

  addSceneNodeAtViewPosition(spUDNode, x, y);
}

void Viewer::addSceneNodeAtViewPosition(UDNodeRef spUDNode, int x, int y)
{
  const Double4x4 &cameraMatrix = spCamera->getMatrix();

  Double4x4 udMat = spUDNode->getUDModel()->getUDMatrix();

  Double3 modelMin = Double3::zero();
  Double3 modelMax = modelMin + Double3{ udMat.axis.x.x, udMat.axis.y.y, udMat.axis.z.z };

  Double3 modelCenter = (modelMax - modelMin) / 2;

  double dist = 2.0;

  // Map mouse coordinates to a point on the plane at y=dist
  Dimensions<int> displayDims = spView->getDimensions();

  double distX, distZ;

  if (spCamera->isOrtho())
  {
    distX = spView->getAspectRatio() * spCamera->getOrthoHeight() / 2;
    distZ = spCamera->getOrthoHeight() / 2;
  }
  else
  {
    // Perspective projection
    distX = dist * spView->getAspectRatio() * Tan(spCamera->getFovY() / 2);
    distZ = dist * Tan(spCamera->getFovY() / 2);
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

  nodePos -= udMat.axis.t;

  Double4x4 nodeMatrix = Double4x4::translation(nodePos.toVector3());

  spUDNode->setMatrix(nodeMatrix);

  spScene->getRootNode()->addChild(spUDNode);
  spScene->makeDirty();
}

void Viewer::staticInit(ep::Kernel *pKernel)
{
  auto spCommandManager = pKernel->getCommandManager();

  spCommandManager->registerCommand("togglebookmarkspanel", Delegate<void(Variant::VarMap)>(&Viewer::staticToggleBookmarksPanel), "", componentID(), "Ctrl+Shift+B");
  // TODO: Bug EP-66
  //spCommandManager->registerCommand("toggleresourcespanel", Delegate<void(Variant::VarMap)>(&Viewer::StaticToggleResourcesPanel), "", componentID(), "Ctrl+Shift+R");
  spCommandManager->registerCommand("createbookmark", Delegate<void(Variant::VarMap)>(&Viewer::staticCreateBookmark), "", componentID(), "Ctrl+B");
}

void Viewer::staticToggleBookmarksPanel(Variant::VarMap params)
{
  Variant *pActivityVar = params.get("activity");
  ViewerRef spViewer = pActivityVar->as<ViewerRef>();

  spViewer->toggleBookmarksPanel();
}

void Viewer::toggleBookmarksPanel()
{
  getUI()->call("togglebookmarkspanel");
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

void Viewer::staticCreateBookmark(Variant::VarMap params)
{
  Variant *pActivityVar = params.get("activity");
  ViewerRef spViewer = pActivityVar->as<ViewerRef>();
  spViewer->createBookmark();
}

void Viewer::createBookmark()
{
  // TODO: Here we have to update the bookmark list separately for the front-end UI and the internal bookmarks list.
  // It would be nice if the QML could automatically update its bookmarks list from the internal bookmarks
  // Not sure how to do this currently
  Variant bookmarkName = spUIBookmarks->call("createbookmark", "");
  spScene->addBookmarkFromCamera(bookmarkName.asString(), spCamera);
}

void Viewer::activate()
{
  getKernel().updatePulse.subscribe(Delegate<void(double)>(this, &Viewer::update));
}

void Viewer::deactivate()
{
  getKernel().updatePulse.unsubscribe(Delegate<void(double)>(this, &Viewer::update));
}

void Viewer::update(double timeStep)
{
  if (spScene)
    spScene->update(timeStep);
}

Variant Viewer::save() const
{
  Variant::VarMap::MapType params;

  if (spCamera)
    params.insert("camera", spCamera->save());

  if (spModel)
  {
    Variant src = spModel->getMetadata()->get("url");
    if (src.is(Variant::Type::String))
    {
      String srcString = src.asString();

      if (!srcString.empty())
        params.insert("model", srcString);
    }
  }

  if (spScene)
    params.insert("scene", spScene->save());

  return Variant(std::move(params));
}

void Viewer::createPlatformLogo()
{
  DataSourceRef spImageSource = pKernel->createDataSourceFromFile("libep/doc/images/platform_logo.png");
  ArrayBufferRef spImage = spImageSource->getResourceAs<ArrayBuffer>("image0");

  // Vertex Shader
  ShaderRef vertexShader = pKernel->createComponent<Shader>();
  {
    vertexShader->setType(ShaderType::VertexShader);

    const char shaderText[] = "attribute vec3 a_position;\n"
      "uniform mat4 u_mfwvp;\n"
      "void main()\n"
      "{\n"
      "  gl_Position = u_mfwvp == mat4(1.0) ? vec4(a_position, 1.0) : vec4(a_position, 1.0);\n"
      "  gl_TexCoord[0].xy = a_position.xy*0.5+0.5;\n"
      "}\n";
    vertexShader->setCode(shaderText);
  }

  // Pixel Shader
  ShaderRef pixelShader = pKernel->createComponent<Shader>();
  {
    pixelShader->setType(ShaderType::PixelShader);

    const char shaderText[] =
      "uniform sampler2D tex;\n"
      "void main()\n"
      "{\n"
      "  vec2 texCoord = gl_TexCoord[0].xy;\n"
      "  texCoord -= .8;\n"
      "  texCoord *= 5.0;\n"
      "  if (texCoord.x < 0.0 || texCoord.y < 0.0) discard;\n"
//       "  texCoord.y = 1 - texCoord.y;\n" // hack to flip the y axis
      "  vec4 col = texture2D(tex, texCoord);\n"
//       "  if (col.a != 1) discard;" // hack to stop flickering
//       "  gl_FragColor = vec4(col.b, col.g, col.r, col.a);\n" // hack to swap r and b
      "  gl_FragColor = col;\n"
      "}\n";
    pixelShader->setCode(shaderText);
  }

  MaterialRef spMaterial = pKernel->createComponent<Material>();
  spMaterial->setShader(ShaderType::VertexShader, vertexShader);
  spMaterial->setShader(ShaderType::PixelShader, pixelShader);

  ArrayBufferRef spVertexBuffer = pKernel->createComponent<ArrayBuffer>();
  ArrayBufferRef spIndexBuffer = pKernel->createComponent<ArrayBuffer>();

  PrimitiveGenerator::generateQuad(spVertexBuffer, spIndexBuffer);

  MetadataRef spMetadata = spVertexBuffer->getMetadata();
  spMetadata->insertAt("a_position", "attributeInfo", 0, "name");

  ModelRef spImageModel = pKernel->createComponent<Model>();

  spImageModel->addVertexArray(spVertexBuffer);
  spImageModel->setIndexArray(spIndexBuffer);
  spImageModel->setMaterial(spMaterial);
  spImageModel->setRenderList(RenderList{ PrimType::Triangles, size_t(0), size_t(0), size_t(6) });

  GeomNodeRef spGeomNode = pKernel->createComponent<GeomNode>();
  spGeomNode->setModel(spImageModel);
  spScene->getRootNode()->addChild(spGeomNode);
  spImageNode = spGeomNode;

  spMaterial->setMaterialProperty("tex", spImage);
}

bool Viewer::inputHook(ep::InputEvent ev)
{
  if (ev.eventType == ep::InputEvent::EventType::Key && ev.deviceType == ep::InputDevice::Keyboard) // if a key event happened on the keyboard
  {
    if (ev.key.state == 1 && ev.key.key == (int)ep::KeyCode::I) // if pressing down on the 'i' key then switch the logos visibility
    {
      NodeRef spRootNode = spScene->getRootNode();
      bool contains = spRootNode->children().exists(spImageNode);

      if (contains)
        spRootNode->removeChild(spImageNode);
      else
        spRootNode->addChild(spImageNode);

      spScene->makeDirty();
      return true; // we return true here so I is not used as input for another action
    }
  }

  return false; // return false so that this can be handled by another action
}

extern "C" bool epPluginAttach()
{
  Kernel::getInstance()->registerComponentType<Viewer>();

#if 0
  // NOTE: Use to test off disk
  Kernel::getInstance()->call("registerQmlComponents", "plugin/viewer/qml");
#else
  Kernel::getInstance()->call("registerQmlComponents", ":/viewer");
#endif

  return true;
}

} // namespace ep
