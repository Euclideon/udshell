#include "components/activities/viewer.h"

#include "renderscene.h"
#include "components/viewport.h"
#include "components/view.h"
#include "components/scene.h"
#include "components/nodes/camera.h"
#include "components/nodes/udnode.h"
#include "components/timer.h"

#include "kernel.h"

namespace ep
{

ComponentDesc Viewer::descriptor =
{
  &Activity::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "viewer", // id
  "Viewer", // displayName
  "Viewer Activity", // description

  nullptr,    // properties
  nullptr,    // methods
  nullptr,    // events
};

Viewer::Viewer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Activity(pType, pKernel, uid, initParams)
{
  // TODO: Remove this once Subscribe returns an identifier for using with Unsubscribe
  updateFunc = Delegate<void(double)>(this, &Viewer::Update);

  UDNodeRef spUDNode = pKernel->CreateComponent<UDNode>();
  ViewRef spView = pKernel->CreateComponent<View>();
  spScene = pKernel->CreateComponent<Scene>();
  spCamera = pKernel->CreateComponent<SimpleCamera>();

  if (spUDNode)
  {
    // TODO: enable streamer once we have a tick running to update the streamer
    spUDNode->Load("data\\DirCube.upc", false);
    //    pUDNode->Load("data\\MCG.uds", true);
    if (!spUDNode->GetSource().empty())
    {
      spScene->GetRootNode()->AddChild(spUDNode);
    }
  }

  udRenderOptions options = { sizeof(udRenderOptions), udRF_None };
  options.flags = udRF_PointCubes | udRF_ClearTargets;
  spView->SetRenderOptions(options);

  spCamera->SetSpeed(1.0);
  spCamera->InvertYAxis(true);
  spCamera->SetPerspective(UD_PIf / 3.f);
  spCamera->SetDepthPlanes(0.0001f, 7500.f);

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
  pKernel->UpdatePulse.Subscribe(updateFunc);
}

void Viewer::Deactivate()
{
  pKernel->UpdatePulse.Unsubscribe(updateFunc);
}

void Viewer::Update(double timeStep)
{
  if (spScene)
    spScene->Update(timeStep);
}

} // namespace ep
