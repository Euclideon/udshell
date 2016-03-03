// kernel includes
#include "ep/cpp/platform.h"
#include "ep/cpp/string.h"
#include "ep/cpp/error.h"
#include "ep/cpp/component/scene.h"
#include "ep/cpp/component/view.h"
#include "ep/cpp/component/node/simplecamera.h"
#include "ep/cpp/component/resource/udmodel.h"
#include "ep/cpp/kernel.h"
#include "helpers.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/component/node/udnode.h"
#include "ep/cpp/component/resourcemanager.h"
#include "components/timer.h"

using namespace kernel;

static void ProcessCmdline(int argc, char *argv[]);

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
static struct
{
  MutableString256 filename;

  uint32_t rendererThreadCount;
  uint32_t streamerMemoryLimit;

  SharedPtr<Kernel> spKernel;

  ViewRef spView;
  UDModelRef spUDModel;
  SceneRef spScene;
  SimpleCameraRef spSimpleCamera;
  UDNodeRef spUDNode;
  TimerRef spCITimer;
  SubscriptionRef spCITimerSub;
  bool CITest;

} mData = {
#if defined(EP_WINDOWS)
               "/src/data/DirCube.uds", // filename
#else
               "~/src/shell/data/DirCube.upc", // filename
#endif

               0,                   // rendererThreadCount
               500 * 1048576,       // streamerMemoryLimit

               nullptr,             // spKernel

               nullptr,             // spView
               nullptr,             // spUDModel
               nullptr,             // spScene
               nullptr,             // spSimpleCamera
               nullptr,             // spUDNode
               nullptr,             // spCITimer
               nullptr,             // spCITimerSub
               false                // CITest
              };

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
static void ViewerInit(String sender, String message, const Variant &data)
{
  using namespace ep;
#if defined(EP_LINUX)
  if (mData.filename.beginsWithIC("~"))
    mData.filename = MutableString256(Format, "{0}{1}", getenv("HOME"), mData.filename.slice(1, mData.filename.length));
#endif // defined(EP_LINUX)
  mData.spView = mData.spKernel->CreateComponent<View>();
  mData.spScene = mData.spKernel->CreateComponent<Scene>();
  mData.spSimpleCamera = mData.spKernel->CreateComponent<SimpleCamera>();
  mData.spUDNode = mData.spKernel->CreateComponent<UDNode>();

  mData.spView->SetUDRenderFlags(UDRenderFlags::PointCubes | UDRenderFlags::ClearTargets);

  mData.spSimpleCamera->SetPosition(Double3::create(0.5, -1.0, 0.5));
  mData.spSimpleCamera->SetSpeed(1.0);
  mData.spSimpleCamera->SetInvertedYAxis(true);
  mData.spSimpleCamera->SetPerspective(EP_PIf / 3.f);
  mData.spSimpleCamera->SetDepthPlanes(0.0001f, 7500.f);

  mData.spView->SetScene(mData.spScene);
  mData.spView->SetCamera(mData.spSimpleCamera);
  mData.spKernel->SetFocusView(mData.spView);

  ResourceManagerRef spResourceManager = mData.spKernel->GetResourceManager();

  if (spResourceManager)
  {
    // TODO: enable streamer once we have a tick running to update the streamer
    DataSourceRef spModelDS = spResourceManager->LoadResourcesFromFile({ { "src", mData.filename },{ "useStreamer", false } });
    if (spModelDS && spModelDS->GetNumResources() > 0)
    {
      mData.spUDModel = spModelDS->GetResourceAs<UDModel>(0);
    }

    mData.spUDNode->SetUDModel(mData.spUDModel);
    mData.spUDNode->SetPosition(Double3::create(0, 0, 0));
  }

  mData.spScene->GetRootNode()->AddChild(mData.spUDNode);
  mData.spScene->MakeDirty();
}

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
static void ViewerDeinit(String sender, String message, const Variant &data)
{
  mData.spScene->GetRootNode()->RemoveChild(mData.spUDNode);

  mData.spUDNode = nullptr;
  mData.spUDModel = nullptr;
  mData.spView = nullptr;
  mData.spScene = nullptr;
  mData.spSimpleCamera = nullptr;
  mData.spCITimerSub = nullptr;
  mData.spCITimer = nullptr;
}

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
int main(int argc, char* argv[])
{
  epInitMemoryTracking();
  if (argc > 1)
  {
    if (String(argv[1]).eqIC("CITest"))
    {
      mData.CITest = true;
      argc = 1;
    }
  }

  using namespace ep;
  mData.rendererThreadCount = epGetHardwareThreadCount() - 1;
  ProcessCmdline(argc, argv);

  epResult result = epR_Failure;
  try
  {
    mData.spKernel = SharedPtr<Kernel>(Kernel::CreateInstance(epParseCommandLine(argc, argv), mData.rendererThreadCount));

    mData.spKernel->RegisterMessageHandler("init", &ViewerInit);
    mData.spKernel->RegisterMessageHandler("deinit", &ViewerDeinit);

    if (mData.CITest)
    {
      mData.spCITimer = mData.spKernel->CreateComponent<Timer>({ { "duration", 4 * 1000 },{ "timertype", "CountDown" } });
      mData.spCITimerSub = mData.spCITimer->Elapsed.Subscribe([]() { Kernel::GetInstance()->Quit(); });
    }

    mData.spKernel->RunMainLoop();

    mData.spKernel = nullptr;
  }
  catch (std::exception &e)
  {
    epDebugFormat("Unhandled exception: {0}\n", e.what());
    result = epR_Failure;
  }
  catch (...)
  {
    epDebugWrite("Unhandled exception!\n");
    result = epR_Failure;
  }

  return result != epR_Success ? 1 : 0;
}

// ---------------------------------------------------------------------------------------
void ProcessCmdline(int argc, char *argv[])
{
  // TODO: port all this code to use udString
  for (int i = 1; i < argc; ++i)
  {
    if (String(argv[i]).beginsWith("/threads="))
    {
      int reqThreads = atoi(&argv[i][9]);
      if (reqThreads > 0)
        mData.rendererThreadCount = reqThreads - 1;
      else
        mData.rendererThreadCount = 0;
    }
    else
    {
      mData.filename = argv[i];
    }
  }
}

// ---------------------------------------------------------------------------------------
#if UDPLATFORM_WINDOWS
// A shim to the standard c main
int __stdcall WinMain(HINSTANCE, HINSTANCE, char *cmdline, int)
{
  // Parse the commandline to handle spaces enclosed in quotes
# define MAX_ARGS 20
  int argc = 0;
  char *argv[MAX_ARGS];

  cmdline = GetCommandLineA(); // This version differs in that it includes the executable
  while (*cmdline)
  {
    char *end;
    if (*cmdline == '"')
    {
      argv[argc++] = ++cmdline;
      end = strchr(cmdline, '"');
    }
    else
    {
      argv[argc++] = cmdline;
      end = strchr(cmdline, ' ');
    }
    if (!end)
      break;
    *end = 0;
    cmdline = end + 1;
    while (*cmdline == ' ')
      ++cmdline;
  }
  int rv = main(argc, argv);
  return rv;
}
#endif // UDPLATFORM_WINDOWS

