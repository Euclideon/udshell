#include "components/activities/viewer.h"

#include "renderscene.h"
#include "components/viewport.h"
#include "components/view.h"
#include "components/scene.h"
#include "components/nodes/camera.h"
#include "components/nodes/udnode.h"
#include "components/timer.h"
#include "components/resources/udmodel.h"
#include "components/datasource.h"
#include "components/resourcemanager.h"

#include "kernel.h"

namespace kernel {

Viewer::Viewer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Activity(pType, pKernel, uid, initParams)
{
  ResourceManagerRef spResourceManager = pKernel->GetResourceManager();

  // TODO: Remove this once Subscribe returns an identifier for using with Unsubscribe
  updateFunc = Delegate<void(double)>(this, &Viewer::Update);

  ViewRef spView = pKernel->CreateComponent<View>();
  spScene = pKernel->CreateComponent<Scene>();

  const Variant *cam = initParams.Get("camera");
  if (cam && cam->is(Variant::Type::AssocArray))
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
      DataSourceRef spModelDS = spResourceManager->LoadResourcesFromFile({ { "src", modelSrc }, { "useStreamer", false } });
      if (spModelDS && spModelDS->GetNumResources() > 0)
      {
        spModel = spModelDS->GetResourceAs<UDModel>(0);
        spCamera->SetPosition(spModel->GetUDMatrix().axis.t.toVector3());
      }
    }
    else if (model->is(Variant::Type::Component))
      spModel = model->as<UDModelRef>();
  }

  if (spModel)
  {
    UDNodeRef spUDNode = pKernel->CreateComponent<UDNode>();
    spUDNode->SetUDModel(spModel);
    spScene->GetRootNode()->AddChild(spUDNode);
  }
  udRenderOptions options = { sizeof(udRenderOptions), udRF_None };
  options.flags = udRF_PointCubes | udRF_ClearTargets;
  spView->SetRenderOptions(options);

  spView->SetScene(spScene);
  spView->SetCamera(spCamera);

  auto spViewport = pKernel->CreateComponent<Viewport>({ { "file", "qrc:/kernel/viewport.qml" }, { "view", spView } });
  if (!spViewport)
  {
    pKernel->LogError("Error creating Viewport Component\n");
    return;
  }

  auto spViewerUI = pKernel->CreateComponent<UIComponent>({ { "file", "qrc:/kernel/activities/viewer/viewer.qml" } });
  if (!spViewerUI)
  {
    pKernel->LogError("Error creating Viewer UI Component\n");
    return;
  }

  spViewerUI->SetProperty("viewport", spViewport);

  ui = spViewerUI;
}

void Viewer::Activate()
{
  GetKernel().UpdatePulse.Subscribe(updateFunc);
}

void Viewer::Deactivate()
{
  GetKernel().UpdatePulse.Unsubscribe(updateFunc);
}

void Viewer::Update(double timeStep)
{
  if (spScene)
    spScene->Update(timeStep);
}

Variant Viewer::Save() const
{
  Array<KeyValuePair> params;

  if (spCamera)
    params.pushBack( KeyValuePair("camera", spCamera->Save()) );

  if (spModel)
    params.pushBack( KeyValuePair("model", spModel->GetDataSource()->GetURL()) );

  return Variant(std::move(params));
}

} // namespace kernel
