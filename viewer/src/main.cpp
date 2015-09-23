// ud includes
#include "udPlatform.h"
#include "udPlatformUtil.h"
#include "udOctree.h"

// kernel includes
#include "ep/epstring.h"
#include "kernel.h"
#include "components/view.h"
#include "components/scene.h"
#include "components/nodes/camera.h"
#include "components/nodes/udnode.h"
#include "eperror.h"

static void ProcessCmdline(int argc, char *argv[]);

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
static struct
{
  udMutableString<256> filename;          // TODO: make this udString

  uint32_t rendererThreadCount;
  uint32_t streamerMemoryLimit;

  ud::Kernel *pKernel;

  ud::ViewRef spView;
  ud::SceneRef spScene;
  ud::SimpleCameraRef spSimpleCamera;
  ud::UDNodeRef spUDNode;
} mData = {
#if UDPLATFORM_WINDOWS
               "/src/data/DirCube.uds", // filename
#else
               "~/src/udshell/data/DirCube.upc", // filename
#endif

               0,                   // rendererThreadCount
               500 * 1048576,       // streamerMemoryLimit

               nullptr,             // pKernel

               nullptr,             // spView
               nullptr,             // spScene
               nullptr              // spSimpleCamera
              };

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
static void ViewerInit(udString sender, udString message, const udVariant &data)
{
  udResult result = udR_Success;
  EPERROR(result);

  using namespace ud;
  //TODO: Error handling for the whole function, perhaps send message with error?
  mData.spView = mData.pKernel->CreateComponent<View>();
  EPERROR_NULL(mData.spView, udR_Failure_, { mData.spView = nullptr; });

  mData.spScene = mData.pKernel->CreateComponent<Scene>();
  EPERROR_NULL(mData.spScene, udR_Failure_, { mData.spScene = nullptr; });

  mData.spSimpleCamera = mData.pKernel->CreateComponent<SimpleCamera>();
  EPERROR_NULL(mData.spSimpleCamera, udR_Failure_, { mData.spSimpleCamera = nullptr; });

  mData.spUDNode = mData.pKernel->CreateComponent<UDNode>();
  EPERROR_NULL(mData.spUDNode, udR_Failure_, { mData.spUDNode = nullptr; });

  udRenderOptions options = { sizeof(udRenderOptions), udRF_None, nullptr, nullptr, nullptr };
  options.flags = udRF_PointCubes | udRF_ClearTargets;
  mData.spView->SetRenderOptions(options);

  mData.spSimpleCamera->SetPosition(udDouble3::create(0.5, -1.0, 0.5));

  mData.spSimpleCamera->SetSpeed(1.0);
  mData.spSimpleCamera->InvertYAxis(true);
  mData.spSimpleCamera->SetPerspective(UD_PIf / 3.f);
  mData.spSimpleCamera->SetDepthPlanes(0.0001f, 7500.f);

  mData.spUDNode->Load(mData.filename, true);
  mData.spUDNode->SetPosition(udDouble3::create(0, 0, 0));

  mData.spView->SetScene(mData.spScene);
  mData.spView->SetCamera(mData.spSimpleCamera);
  mData.pKernel->SetFocusView(mData.spView);

  if (!mData.spUDNode->GetSource().empty())
    mData.spScene->GetRootNode()->AddChild(mData.spUDNode);

  mData.spScene->MakeDirty();
}

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
udResult ViewerDeinit()
{
  mData.spScene->GetRootNode()->RemoveChild(mData.spUDNode);

  mData.spUDNode = nullptr;
  mData.spView = nullptr;
  mData.spScene = nullptr;
  mData.spSimpleCamera = nullptr;

  mData.pKernel->Destroy();
  mData.pKernel = nullptr;

  udOctree_Shutdown();

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
int main(int argc, char* argv[])
{
  udMemoryDebugTrackingInit();

  using namespace ud;
  mData.rendererThreadCount = udGetHardwareThreadCount() - 1;
  ProcessCmdline(argc, argv);

  udResult result = ud::Kernel::Create(&mData.pKernel, udParseCommandLine(argc, argv), mData.rendererThreadCount);
  if (result != udR_Success)
    return -1;

  mData.pKernel->RegisterMessageHandler("init", &ViewerInit);

  if (mData.pKernel->RunMainLoop() != udR_Success)
  {
    // TODO: improve error handling/reporting
    udDebugPrintf("Error encountered in Kernel::RunMainLoop()\n");
    return 1;
  }

  result = ViewerDeinit();

  udMemoryOutputLeaks();
  udMemoryDebugTrackingDeinit();

  if (result != udR_Success)
    return -1;

  return 0;
}

// ---------------------------------------------------------------------------------------
void ProcessCmdline(int argc, char *argv[])
{
  // TODO: port all this code to use udString
  for (int i = 1; i < argc; ++i)
  {
    if (udStrBeginsWith(argv[i], "/threads="))
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

void update(ud::ViewRef spView, ud::SceneRef spScene)
{
  using namespace ud;
  int displayWidth, displayHeight;
  mData.spView->GetDimensions(&displayWidth, &displayHeight);

//   mData.spSimpleCamera->ForceDirty();
//   mData.spScene->ForceDirty();
}

