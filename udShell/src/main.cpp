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

using namespace ud;

Kernel *s_pKernel = nullptr;


// ---------------------------------------------------------------------------------------
void DbgMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if (s_pKernel)
  {
    // TODO: replace this with something more robust - maybe a full logging system and status console
    switch (type) {
      case QtDebugMsg:
        s_pKernel->LogDebug(2, udSharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toLatin1().data(), context.file, context.line, context.function));
        break;
      case QtWarningMsg:
        s_pKernel->LogWarning(2, udSharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toLatin1().data(), context.file, context.line, context.function));
        break;
      case QtCriticalMsg:
      case QtFatalMsg:
        s_pKernel->LogError(udSharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toLatin1().data(), context.file, context.line, context.function));
    }
  }
  else
  {
    udDebugPrintf("Qt Dbg: %s (%s:%d, %s)\n", msg.toLatin1().data(), context.file, context.line, context.function);
  }
}

void Init(udString sender, udString message, const udVariant &data)
{
  // TODO: load a project file...

  auto spView = s_pKernel->CreateComponent<View>();
  auto spScene = s_pKernel->CreateComponent<Scene>();
  auto spCamera = s_pKernel->CreateComponent<SimpleCamera>();
  auto spUDNode = s_pKernel->CreateComponent<UDNode>();

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

  auto spMainWindow = s_pKernel->CreateComponent<Window>({ { "file", "qrc:/qml/window.qml" } });
  if (!spMainWindow)
  {
    s_pKernel->LogError("Error creating MainWindow UI Component\n");
    return;
  }

  auto spViewport = s_pKernel->CreateComponent<Viewport>({ { "file", "qrc:/kernel/viewport.qml" }, { "view", spView } });
  if (!spViewport)
  {
    s_pKernel->LogError("Error creating Main Viewport Component\n");
    return;
  }

  spMainWindow->SetTopLevelUI(spViewport);
}

// ---------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  udMemoryDebugTrackingInit();

  // unit test
  udResult udSlice_Test();
  udSlice_Test();
  udResult udString_Test();
  udString_Test();

  // install our qt message handler
  qInstallMessageHandler(DbgMessageHandler);
  //QLoggingCategory::setFilterRules("qt.*=true");

  // TEMP: this is just for testing - force qt to use the threaded renderer on windows
  // 5.5 uses this by default
  qputenv("QSG_RENDER_LOOP", "threaded");

  // create a kernel
  udResult r = Kernel::Create(&s_pKernel, udParseCommandLine(argc, argv), 8);
  if (r == udR_Failure_)
  {
    // TODO: improve error handling/reporting
    udDebugPrintf("Error creating Kernel\n");
    return 1;
  }

  s_pKernel->RegisterMessageHandler("init", &Init);

  if (s_pKernel->RunMainLoop() != udR_Success)
  {
    // TODO: improve error handling/reporting
    udDebugPrintf("Error encountered in Kernel::RunMainLoop()\n");
    return 1;
  }

  udMemoryOutputLeaks();
  udMemoryDebugTrackingDeinit();

  return 0;
}
