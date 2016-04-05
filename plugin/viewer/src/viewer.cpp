#include "viewer.h"

#include "ep/cpp/component/viewport.h"
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

  Variant::VarMap sceneParams;
  const Variant *pSceneParams = initParams.Get("scene");
  if (pSceneParams && pSceneParams->is(Variant::SharedPtrType::AssocArray))
    sceneParams = pSceneParams->asAssocArray();
  spScene = pKernel->CreateComponent<Scene>(sceneParams);

  Variant::VarMap cameraParams;
  const Variant *pCam = initParams.Get("camera");
  if (pCam && pCam->is(Variant::SharedPtrType::AssocArray))
    cameraParams = pCam->asAssocArray();
  spCamera = pKernel->CreateComponent<SimpleCamera>(Variant::VarMap(cameraParams));

  const Variant *model = initParams.Get("model");
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

  spView->SetUDRenderFlags(UDRenderFlags::PointCubes | UDRenderFlags::ClearTargets);

  spView->SetScene(spScene);
  spView->SetCamera(spCamera);

  ViewportRef spViewport;
  epscope(fail) { if (!spViewport) pKernel->LogError("Error creating Viewport Component\n"); };
  spViewport = pKernel->Call("createqmlcomponent", "ep.viewport", "qrc:/kernel/viewport.qml", Variant::VarMap{ { "view", spView } }).as<ViewportRef>();

  UIComponentRef spViewerUI;
  epscope(fail) { if(!spViewerUI) pKernel->LogError("Error creating Viewer UI Component\n"); };
  spViewerUI = pKernel->Call("createqmlcomponent", "ep.uicomponent", "qrc:/viewer/viewer.qml", nullptr).as<UIComponentRef>();
  spViewerUI->Set("viewport", spViewport);
  spViewerUI->Subscribe("resourcedropped", Delegate<void(String, int, int)>(this, &Viewer::OnResourceDropped));

  epscope(fail) { if(!spUIBookmarks) pKernel->LogError("Error creating bookmarks UI Component\n"); };
  spUIBookmarks = pKernel->Call("createqmlcomponent", "ep.uicomponent", "qrc:/qml/components/bookmarksui.qml", nullptr).as<UIComponentRef>();
  spUIBookmarks->Set("view", spView);
  spViewerUI->Set("bookmarkscomp", spUIBookmarks);

  UIComponentRef spUIResources;
  ComponentRef spComp = pKernel->FindComponent("resourcespanel");
  if (spComp)
    spUIResources = component_cast<UIComponent>(spComp);
  else
  {
    epscope(fail) { if (!spUIResources) pKernel->LogError("Error creating Resource Panel UI Component\n"); };
    spUIResources = pKernel->Call("createqmlcomponent", "ep.uicomponent", "qrc:/qml/components/resourcespanel.qml", Variant::VarMap{ { "name", "resourcespanel" } }).as<UIComponentRef>();
  }
  spViewerUI->Set("resourcespanel", spUIResources);

  SetUI(spViewerUI);

  // Add bookmarks to UI
  auto bmMap = spScene->GetBookmarkMap();
  for (auto bm : bmMap)
    spUIBookmarks->Call("createbookmark", bm.key);
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

  try {
    spUDModel = spResourceManager->GetResourceAs<UDModel>(resourceUID);
  }
  catch(EPException &) {
    LogDebug(2, "Can't drop \"{0}\" into viewport -- Component type is not supported", resourceUID);
    ClearError();
    return;
  }

  UDNodeRef spUDNode = pKernel->CreateComponent<UDNode>();
  spUDNode->SetUDModel(spUDModel);

  AddSceneNodeAtViewPosition(spUDNode, x, y);
}

void Viewer::AddSceneNodeAtViewPosition(UDNodeRef spUDNode, int x, int y)
{
  const Double4x4 &cameraMatrix = spCamera->GetMatrix();

  BoundingVolume vol = spUDNode->GetUDModel()->GetBoundingVolume();
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
  spCommandManager->RegisterCommand("toggleresourcespanel", Delegate<void(Variant::VarMap)>(&Viewer::StaticToggleResourcesPanel), "", ComponentID(), "Ctrl+Shift+R");
  spCommandManager->RegisterCommand("createbookmark", Delegate<void(Variant::VarMap)>(&Viewer::StaticCreateBookmark), "", ComponentID(), "Ctrl+B");
}

void Viewer::StaticToggleBookmarksPanel(Variant::VarMap params)
{
  Variant *pActivityVar = params.Get("activity");
  ViewerRef spViewer = pActivityVar->as<ViewerRef>();

  spViewer->ToggleBookmarksPanel();
}

void Viewer::ToggleBookmarksPanel()
{
  GetUI()->Call("togglebookmarkspanel");
}

void Viewer::StaticToggleResourcesPanel(Variant::VarMap params)
{
  Variant *pActivityVar = params.Get("activity");
  ViewerRef spViewer = pActivityVar->as<ViewerRef>();

  spViewer->ToggleResourcesPanel();
}

void Viewer::ToggleResourcesPanel()
{
  GetUI()->Call("toggleresourcespanel");
}

void Viewer::StaticCreateBookmark(Variant::VarMap params)
{
  Variant *pActivityVar = params.Get("activity");
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
    params.Insert("camera", spCamera->Save());

  if (spModel)
  {
    Variant src = spModel->GetMetadata()->Get("url");
    if (src.is(Variant::Type::String))
    {
      String srcString = src.asString();

      if (!srcString.empty())
        params.Insert("model", srcString);
    }
  }

  if (spScene)
    params.Insert("scene", spScene->Save());

  return Variant(std::move(params));
}

extern "C" bool epPluginAttach()
{
  Kernel::GetInstance()->RegisterComponentType<Viewer>();

  return true;
}

} // namespace ep
