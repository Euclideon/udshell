#include <QLoggingCategory>

#include "kernel.h"
#include "renderscene.h"
#include "components/view.h"
#include "components/scene.h"
#include "components/nodes/camera.h"
#include "components/nodes/udnode.h"
#include "components/timer.h"
#include "hal/debugfont.h"

using namespace ud;

Kernel *s_pKernel;


// ---------------------------------------------------------------------------------------
void DbgMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  // TODO: replace this with something more robust - maybe a full logging system and status console
  switch (type) {
  case QtDebugMsg:
    udDebugPrintf("Dbg: %s (%s:%d, %s)\n", msg.toLatin1().data(), context.file, context.line, context.function);
    break;
  case QtWarningMsg:
    udDebugPrintf("Wrn: %s (%s:%d, %s)\n", msg.toLatin1().data(), context.file, context.line, context.function);
    break;
  case QtCriticalMsg:
    udDebugPrintf("Crt: %s (%s:%d, %s)\n", msg.toLatin1().data(), context.file, context.line, context.function);
    break;
  case QtFatalMsg:
    udDebugPrintf("Ftl: %s (%s:%d, %s)\n", msg.toLatin1().data(), context.file, context.line, context.function);
    abort();
  }
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
    udDebugPrintf("Error creating Kernel\n");
    return 1;
  }

  auto pView = s_pKernel->CreateComponent<View>();
  auto pScene = s_pKernel->CreateComponent<Scene>();
  auto pCamera = s_pKernel->CreateComponent<SimpleCamera>();
  auto pUDNode = s_pKernel->CreateComponent<UDNode>();

  if (pUDNode)
  {
    // TODO: enable streamer once we have a tick running to update the streamer
    pUDNode->Load("data\\DirCube.upc", false);
//    pUDNode->Load("data\\MCG.uds", true);
    if (!pUDNode->GetSource().empty())
    {
      pScene->GetRootNode()->AddChild(pUDNode);
    }
  }

  udRenderOptions options = { sizeof(udRenderOptions), udRF_None };
  options.flags = udRF_PointCubes | udRF_ClearTargets;
  pView->SetRenderOptions(options);

  pCamera->SetSpeed(1.0);
  pCamera->InvertYAxis(true);
  pCamera->SetPerspective(UD_PIf / 3.f);
  pCamera->SetDepthPlanes(0.0001f, 7500.f);

  pView->SetScene(pScene);
  pView->SetCamera(pCamera);
  s_pKernel->SetFocusView(pView);

  udSlice<const udKeyValuePair> params = { {"file", "qrc:/qml/main.qml"} };
  auto spMainWindow = s_pKernel->CreateComponent<UIComponent>(params);
  if (!spMainWindow)
  {
    udDebugPrintf("Error creating MainWindow UI Component\n");
    return 1;
  }
  s_pKernel->FormatMainWindow(spMainWindow);

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
