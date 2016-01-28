#include "viewer.h"

#include "ep/cpp/component/viewport.h"
#include "ep/cpp/component/view.h"
#include "ep/cpp/component/scene.h"
#include "ep/cpp/component/node/simplecamera.h"
#include "ep/cpp/component/node/udnode.h"
#include "ep/cpp/component/resource/udmodel.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/component/resourcemanager.h"
#include "ep/cpp/component/commandmanager.h"

namespace ep {

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

  const Variant *cam = initParams.Get("camera");
  if (cam && cam->is(Variant::SharedPtrType::AssocArray))
  {
    Variant::VarMap cameraParams = cam->asAssocArray();
    spCamera = pKernel->CreateComponent<SimpleCamera>(Variant::VarMap(cameraParams));
  }

  const Variant *model = initParams.Get("model");
  if (model)
  {
    if (model->is(Variant::Type::String))
    {
      // TODO: enable streamer once we have a tick running to update the streamer
      String modelSrc = model->asString();
      DataSourceRef spModelDS = spResourceManager->LoadResourcesFromFile({ { "src", modelSrc }, { "useStreamer", true } });
      if (spModelDS && spModelDS->GetNumResources() > 0)
      {
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
    spView->SetEnablePicking(true);
    spScene->AddBookmark(MutableString128(Format, "{0}_bookmark", model->asString().getRightAtLast("/", false)), { spModel->GetUDMatrix().axis.t.toVector3(), { 0, 0, 0 }});
  }

  spView->SetUDRenderFlags(UDRenderFlags::PointCubes | UDRenderFlags::ClearTargets);

  spView->SetScene(spScene);
  spView->SetCamera(spCamera);

  auto spViewport = pKernel->CreateComponent<Viewport>({ { "file", "qrc:/kernel/viewport.qml" }, { "view", spView } });
  if (!spViewport)
  {
    pKernel->LogError("Error creating Viewport Component\n");
    throw epR_Failure;
  }

  auto spViewerUI = pKernel->CreateComponent<UIComponent>({ { "file", "qrc:/viewer/viewer.qml" } });
  if (!spViewerUI)
  {
    pKernel->LogError("Error creating Viewer UI Component\n");
    throw epR_Failure;
  }
  spViewerUI->SetProperty("viewport", spViewport);

  spUIBookmarks = pKernel->CreateComponent<UIComponent>({ { "file", "qrc:/qml/bookmarksui.qml" } });
  if (!spUIBookmarks)
  {
    pKernel->LogError("Error creating bookmarks UI Component\n");
    throw epR_Failure;
  }
  spUIBookmarks->SetProperty("view", spView);

  spViewerUI->SetProperty("bookmarkscomp", spUIBookmarks);

  SetUI(spViewerUI);

  // Add bookmarks to UI
  auto bmMap = spScene->GetBookmarkMap();
  for (auto bm : bmMap)
    spUIBookmarks->CallMethod("createbookmark", bm.key);
}

void Viewer::StaticInit(ep::Kernel *pKernel)
{
  auto spCommandManager = pKernel->GetCommandManager();

  spCommandManager->RegisterCommand("togglebookmarkspanel", Delegate<void(Variant::VarMap)>(&Viewer::StaticToggleBookmarksPanel), "", ComponentID(), "Ctrl+Shift+B");
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
  GetUI()->CallMethod("togglebookmarkspanel");
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
  Variant bookmarkName = spUIBookmarks->CallMethod("createbookmark", "");
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
    params.Insert("model", spModel->GetDataSource()->GetURL());

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
