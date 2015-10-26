#include <QLoggingCategory>

#include "kernel.h"
#include "renderscene.h"
#include "components/logger.h"
#include "components/window.h"
#include "components/viewport.h"
#include "components/view.h"
#include "components/scene.h"
#include "components/nodes/camera.h"
#include "components/nodes/udnode.h"
#include "components/timer.h"
#include "hal/debugfont.h"

static Kernel *pKernel = nullptr;
static WindowRef spMainWindow;

// ---------------------------------------------------------------------------------------
void DbgMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if (pKernel)
  {
    // TODO: replace this with something more robust - maybe a full logging system and status console
    switch (type) {
      case QtDebugMsg:
        pKernel->LogDebug(2, SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
      case QtWarningMsg:
        pKernel->LogWarning(2, SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
      case QtCriticalMsg:
      case QtFatalMsg:
        pKernel->LogError(SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
    }
  }
  else
  {
    udDebugPrintf("Qt Dbg: %s (%s:%d, %s)\n", msg.toUtf8().data(), context.file, context.line, context.function);
  }
}

void Init(String sender, String message, const Variant &data)
{
  // TODO: load a project file...

  auto spView = pKernel->CreateComponent<View>();
  auto spScene = pKernel->CreateComponent<Scene>();
  auto spCamera = pKernel->CreateComponent<SimpleCamera>();
  auto spUDNode = pKernel->CreateComponent<UDNode>();

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

  spMainWindow = pKernel->CreateComponent<Window>({ { "file", "qrc:/qml/window.qml" } });
  if (!spMainWindow)
  {
    pKernel->LogError("Error creating MainWindow UI Component\n");
    return;
  }

  auto spViewport = pKernel->CreateComponent<Viewport>({ { "file", "qrc:/kernel/viewport.qml" }, { "view", spView } });
  if (!spViewport)
  {
    pKernel->LogError("Error creating Main Viewport Component\n");
    return;
  }

  spMainWindow->SetTopLevelUI(spViewport);
}

void Deinit(String sender, String message, const Variant &data)
{
  spMainWindow = nullptr;
}

// ---------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  udMemoryDebugTrackingInit();

  // unit test
  epSlice_Test();
  epString_Test();
  epSharedPtr_Test();

  // install our qt message handler
  qInstallMessageHandler(DbgMessageHandler);
  //QLoggingCategory::setFilterRules("qt.*=true");

  // TEMP: this is just for testing - force qt to use the threaded renderer on windows
  // 5.5 uses this by default
  qputenv("QSG_RENDER_LOOP", "threaded");

  // create a kernel
  epResult r = Kernel::Create(&pKernel, udParseCommandLine(argc, argv), 8);
  if (r == epR_Failure_)
  {
    // TODO: improve error handling/reporting
    udDebugPrintf("Error creating Kernel\n");
    return 1;
  }

  pKernel->RegisterMessageHandler("init", &Init);
  pKernel->RegisterMessageHandler("deinit", &Deinit);

  if (pKernel->RunMainLoop() != udR_Success)
  {
    // TODO: improve error handling/reporting
    udDebugPrintf("Error encountered in Kernel::RunMainLoop()\n");
    return 1;
  }

  udMemoryOutputLeaks();
  udMemoryDebugTrackingDeinit();

  return 0;
}
