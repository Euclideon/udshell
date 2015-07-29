#include <QLoggingCategory>

#include "kernel.h"
#include "renderscene.h"
#include "components/view.h"
#include "components/scene.h"
#include "components/nodes/camera.h"
#include "components/nodes/udnode.h"
#include "hal/debugfont.h"
#include "quick/window.h"

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


void update(ViewRef pView, SceneRef pScene)
{
  udDebugPrintf("main - update()\n");

  CameraRef pCamera = pView->GetCamera();
  pCamera->SetPerspective(UD_PIf / 3.f);
  pCamera->SetDepthPlanes(0.0001f, 7500.f);
  //pCamera->Update();

  udRenderOptions options = { sizeof(udRenderOptions), udRF_None };
  options.flags = udRF_PointCubes | udRF_ClearTargets;

  pView->SetRenderOptions(options);
  pView->ForceDirty();
  pScene->ForceDirty();
}

void display(ViewRef pView, SceneRef pScene)
{
  udDebugFont_BeginRender();

  udDebugConsole_SetCursorPos(10.f, 20.f);
  udDebugConsole_SetTextScale(0.4f);
  udDebugConsole_Printf("TEST!!!!!");
}

// ---------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
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
    return 1;

  auto pView = s_pKernel->CreateComponent<View>();
  auto pScene = s_pKernel->CreateComponent<Scene>();
  auto pCamera = s_pKernel->CreateComponent<SimpleCamera>();
  auto pUDNode = s_pKernel->CreateComponent<UDNode>();

  if (pUDNode)
  {
    // TODO: enable streamer once we have a tick running to update the streamer
    pUDNode->Load("res\\DirCube.upc", false);
    if (!pUDNode->GetSource().empty())
    {
      pScene->GetRootNode()->AddChild(pUDNode);
    }
  }

  pView->RegisterPreRenderCallback(update);
  pView->RegisterPostRenderCallback(display);

  pCamera->SetSpeed(1.0);
  pCamera->InvertYAxis(true);

  pView->SetScene(pScene);
  pView->SetCamera(pCamera);
  s_pKernel->SetFocusView(pView);

  // create the main window
  // TODO: make it more obvious that kernel is taking ownership of our window??
  Window *w = new Window;
  w->setResizeMode(QQuickView::SizeRootObjectToView);
  w->setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  w->resize(800, 600);
  w->show();
  w->raise();

  return s_pKernel->RunMainLoop();
}
