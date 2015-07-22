#include <QLoggingCategory>

#include "udKernel.h"
#include "udView.h"
#include "udRenderScene.h"
#include "udScene.h"
#include "udCamera.h"
#include "udDebugFont.h"

#include "quick/window.h"

udKernel *s_pKernel;


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

static udFilename s_filename("d:\\GIS_Example.uds");
static udOctree *pMainModel = 0;
static uint32_t streamingTime = 0;
udDouble4x4 s_mat = udDouble4x4::identity();

void LoadModel()
{
  udOctree_Destroy(&pMainModel);

  udDebugPrintf("Loading %s\n", s_filename.GetFilenameWithExt());
  streamingTime = udGetTimeMs();

  if (udOctree_Create(&pMainModel, s_filename, nullptr, 0) != udR_Success)
  {
    udDebugPrintf("Failed to load model\n");
  }
  else
  {
    udDebugPrintf("Loaded model: %s\n", s_filename.GetFilenameWithExt());
    int metadataCount;
    udOctree_GetMetadataCount(pMainModel, &metadataCount);
    udDebugPrintf("%d metadata entries\n", metadataCount);
    for (int i = 0; i < metadataCount; ++i)
    {
      const char *pName;
      const char *pValue;
      udOctree_GetMetadataByIndex(pMainModel, i, &pName, &pValue);
      udDebugPrintf("  '%s' = '%s'\n", pName, pValue);
    }
  }
}

void update(udViewRef pView, udSceneRef pScene)
{
  udDebugPrintf("main - update()\n");

  udCameraRef pCamera = pView->GetCamera();
  pCamera->SetPerspective(UD_PIf / 3.f);
  pCamera->SetDepthPlanes(0.0001f, 7500.f);
  //pCamera->Update();

  udRenderModel renderModel = { 0 };
  udRenderOptions options = { sizeof(udRenderOptions), udRF_None };
  options.flags = udRF_PointCubes | udRF_ClearTargets;

  if (pMainModel)
  {
    renderModel.pOctree = pMainModel;
    renderModel.pClip = nullptr;
    renderModel.flags = udRF_PointCubes;
    renderModel.pWorldMatrixD = (const double *)&s_mat;
  }

  pScene->SetRenderOptions(options);
  pScene->SetRenderModels(&renderModel, 1);
}

void display(udViewRef pView, udSceneRef pScene)
{
  udDebugPrintf("main - display()\n");

  int displayWidth, displayHeight;
  pView->GetDimensions(&displayWidth, &displayHeight);

  int renderWidth, renderHeight;
  pView->GetRenderDimensions(&renderWidth, &renderHeight);

  //udRenderView *pRenderView = pView->GetRenderableView()->GetRenderView();
  //udCameraRef pCamera = pView->GetCamera();

  udStreamerStatus streamerStatus = { 0 };
  //s_pKernel->GetStreamer()->Update(&streamerStatus);

  udOctree_Update(&streamerStatus);

  if (!streamerStatus.active && streamingTime)
  {
    udDebugPrintf("%s streamed in %1.2f seconds (from load to inactive)\n", s_filename.GetFilenameWithExt(), (udGetTimeMs() - streamingTime) / 1000.f);
    streamingTime = 0;
  }

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

  // create a kernel
  udResult r = udKernel::Create(&s_pKernel, udParseCommandLine(argc, argv), 8);
  if (r == udR_Failure_)
    return 1;

  auto pView = s_pKernel->CreateComponent<udView>();
  auto pScene = s_pKernel->CreateComponent<udScene>();
  auto pCamera = s_pKernel->CreateComponent<udSimpleCamera>();

  pView->RegisterPreRenderCallback(update);
  pView->RegisterPostRenderCallback(display);

  pCamera->SetSpeed(1.0);
  pCamera->InvertYAxis(true);

  pView->SetScene(pScene);
  pView->SetCamera(pCamera);
  s_pKernel->SetFocusView(pView);

  //LoadModel();

  // create the main window
  Window w;
  w.setResizeMode(QQuickView::SizeRootObjectToView);
  w.setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  w.resize(800, 600);
  w.show();
  w.raise();

  return s_pKernel->RunMainLoop();
}
