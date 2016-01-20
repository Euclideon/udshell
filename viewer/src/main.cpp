// kernel includes
#include "ep/cpp/platform.h"
#include "ep/cpp/string.h"
#include "ep/cpp/error.h"
#include "ep/cpp/component/scene.h"
#include "ep/cpp/component/node/simplecamera.h"
#include "ep/cpp/component/resource/udmodel.h"
#include "kernel.h"
#include "helpers.h"
#include "components/viewimpl.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/component/node/udnode.h"
#include "components/resourcemanager.h"

using namespace kernel;

static void ProcessCmdline(int argc, char *argv[]);

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
static struct
{
  MutableString256 filename;          // TODO: make this udString

  uint32_t rendererThreadCount;
  uint32_t streamerMemoryLimit;

  kernel::Kernel *pKernel;

  ViewRef spView;
  UDModelRef spUDModel;
  SceneRef spScene;
  SimpleCameraRef spSimpleCamera;
  UDNodeRef spUDNode;
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
static void ViewerInit(String sender, String message, const Variant &data)
{
  using namespace ep;

  //TODO: Error handling for the whole function, perhaps send message with error?
  mData.spView = mData.pKernel->CreateComponent<View>();
  EPTHROW_IF_NULL(mData.spView, epR_Failure, "!");
  epscope(fail) { mData.spView = nullptr; };

  mData.spScene = mData.pKernel->CreateComponent<Scene>();
  EPTHROW_IF_NULL(mData.spScene, epR_Failure, "!");
  epscope(fail) { mData.spScene = nullptr; };

  mData.spSimpleCamera = mData.pKernel->CreateComponent<SimpleCamera>();
  EPTHROW_IF_NULL(mData.spSimpleCamera, epR_Failure, "!");
  epscope(fail) { mData.spSimpleCamera = nullptr; };

  mData.spUDNode = mData.pKernel->CreateComponent<UDNode>();
  EPTHROW_IF_NULL(mData.spUDNode, epR_Failure, "!");
  epscope(fail) { mData.spUDNode = nullptr; };

  udRenderOptions options = { sizeof(udRenderOptions), udRF_None, nullptr, nullptr, nullptr };
  options.flags = udRF_PointCubes | udRF_ClearTargets;
  mData.spView->GetImpl<ViewImpl>()->SetRenderOptions(options);

  mData.spSimpleCamera->SetPosition(Double3::create(0.5, -1.0, 0.5));

  mData.spSimpleCamera->SetSpeed(1.0);
  mData.spSimpleCamera->SetInvertedYAxis(true);
  mData.spSimpleCamera->SetPerspective(EP_PIf / 3.f);
  mData.spSimpleCamera->SetDepthPlanes(0.0001f, 7500.f);

  mData.spView->SetScene(mData.spScene);
  mData.spView->SetCamera(mData.spSimpleCamera);
  mData.pKernel->SetFocusView(mData.spView);

  ResourceManagerRef spResourceManager = mData.pKernel->GetResourceManager();

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
}

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
int main(int argc, char* argv[])
{
  using namespace ep;
  mData.rendererThreadCount = epGetHardwareThreadCount() - 1;
  ProcessCmdline(argc, argv);

  epResult result = epR_Failure;
  try
  {
    result = kernel::Kernel::Create(&mData.pKernel, udParseCommandLine(argc, argv), mData.rendererThreadCount);
    if (result != epR_Success)
      return -1;

    mData.pKernel->RegisterMessageHandler("init", &ViewerInit);
    mData.pKernel->RegisterMessageHandler("deinit", &ViewerDeinit);

    if (mData.pKernel->RunMainLoop() != epR_Success)
    {
      // TODO: improve error handling/reporting
      epDebugPrintf("Error encountered in Kernel::RunMainLoop()\n");
      return 1;
    }

    mData.pKernel->Destroy();
    mData.pKernel = nullptr;
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

void update(ViewRef spView, SceneRef spScene)
{
  using namespace ep;
  Dimensions<int> displaySize = mData.spView->GetDimensions();

//   mData.spSimpleCamera->ForceDirty();
//   mData.spScene->ForceDirty();
}

