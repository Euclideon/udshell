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

#include "ep/cpp/component/timer.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/resource/material.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/resource/shader.h"
#include "ep/cpp/component/scene.h"
#include "ep/cpp/component/resource/metadata.h"
#include "ep/cpp/component/node/geomnode.h"
#include "dinkey.h"
#include "ep/cpp/delegate.h"

using namespace ep;

struct Vertex
{
  Float3 position;
  Float4 color;
};

template<>
ep::SharedString stringof<Vertex>()
{
  return "{f32[3], f32[4]}";
}

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
  GeomNodeRef spTestGeomNode;
  TimerRef spCITimer;
  SubscriptionRef spCITimerSub;
  SubscriptionRef spUpdateSub;
  bool shutdownTest;
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
               nullptr,             // spTestGeomNode
               nullptr,             // spCITimer
               nullptr,             // spCITimerSub
               nullptr,             // spUpdateSub
               false                // shutdownTest
              };

#if EP_DEBUG
static GeomNodeRef CreateTestModel(KernelRef kernel)
{
  // Vertex Shader
  ShaderRef vertexShader = kernel->CreateComponent<Shader>();
  {
    vertexShader->SetType(ShaderType::VertexShader);

    const char shaderText[] = "attribute vec3 a_position;\n"
                              "attribute vec4 a_color;\n"
                              "varying vec4 v_color;\n"
                              "uniform mat4 u_mfwvp;\n"
                              "void main()\n"
                              "{\n"
                              "  v_color = a_color;\n"
                              "  gl_Position = u_mfwvp * vec4(a_position, 1.0);\n"
                              "}\n";
    vertexShader->SetCode(shaderText);
  }

  // Pixel Shader
  ShaderRef pixelShader = kernel->CreateComponent<Shader>();
  {
    pixelShader->SetType(ShaderType::PixelShader);

    const char shaderText[] = "varying vec4 v_color;\n"
                              "void main()\n"
                              "{\n"
                              "  gl_FragColor = v_color;\n"
                              "}\n";
    pixelShader->SetCode(shaderText);
  }

  // Material
  MaterialRef material = kernel->CreateComponent<Material>();
  material->SetShader(ShaderType::VertexShader, vertexShader);
  material->SetShader(ShaderType::PixelShader, pixelShader);
#if 0
  material->SetTexture(0, texture);
#endif // 0

  // Vertex Buffer
  ArrayBufferRef vertexBuffer = kernel->CreateComponent<ArrayBuffer>();
  {

    static const Vertex vb[] = { Vertex{ Float3{ 0.0f, 0.0f, 0.0f },   Float4{ 1.0f, 1.0f, 1.0f, 1.0f } },
                                 Vertex{ Float3{ 1.0f, 0.0f, 0.0f },   Float4{ 1.0f, 0.0f, 0.0f, 1.0f } },
                                 Vertex{ Float3{ 1.0f, 1.0f, 0.0f },   Float4{ 0.0f, 1.0f, 0.0f, 1.0f } },
                                 Vertex{ Float3{ 0.0f, 1.0f, 0.0f },   Float4{ 0.0f, 0.0f, 1.0f, 1.0f } },

                                 Vertex{ Float3{ 0.0f, 0.0f, 1.0f },   Float4{ 1.0f, 1.0f, 0.0f, 1.0f } },
                                 Vertex{ Float3{ 1.0f, 0.0f, 1.0f },   Float4{ 1.0f, 0.0f, 1.0f, 1.0f } },
                                 Vertex{ Float3{ 1.0f, 1.0f, 1.0f },   Float4{ 0.0f, 1.0f, 1.0f, 1.0f } },
                                 Vertex{ Float3{ 0.0f, 1.0f, 1.0f },   Float4{ 0.5f, 0.5f, 1.0f, 1.0f } } };

    vertexBuffer->AllocateFromData(Slice<const Vertex>(vb));
    MetadataRef metadata = vertexBuffer->GetMetadata();
    metadata->Get("attributeinfo")[0].insertItem("name", "a_position");
    metadata->Get("attributeinfo")[1].insertItem("name", "a_color");
  }

  // Index Buffer
  ArrayBufferRef indexBuffer = kernel->CreateComponent<ArrayBuffer>();
  static const uint16_t ib[] = {
                                 0, 4, 5,
                                 0, 5, 1,

                                 1, 5, 6,
                                 1, 6, 2,

                                 2, 6, 7,
                                 2, 7, 3,

                                 3, 7, 4,
                                 3, 4, 0,

                                 4, 7, 6,
                                 4, 6, 5,

                                 3, 0, 1,
                                 3, 1, 2
                              };
  indexBuffer->AllocateFromData(Slice<const uint16_t>(ib));

#if 0
  // Texture
  ArrayBufferRef texture = kernel->CreateComponent<ArrayBuffer>();
  {
    texture->Allocate("White", sizeof(uint32_t), { 32, 32 });
    Slice<void> data  = texture->Map();
    memset(data.ptr, 0xFF, data.length);
    texture->Unmap();
  }
#endif // 0

  ModelRef model = kernel->CreateComponent<Model>();
  model->setName("TestModel");
  model->AddVertexArray(vertexBuffer);
  model->SetIndexArray(indexBuffer);
  model->SetMaterial(material);
  model->SetRenderList(RenderList { PrimType::Triangles, size_t(0), size_t(0), size_t(EPARRAYSIZE(ib)) });

  GeomNodeRef geomNode = kernel->CreateComponent<GeomNode>();
  geomNode->SetModel(model);

  return geomNode;
}
#endif // EP_DEBUG

void Update(double delta)
{
  if (mData.spScene)
    mData.spScene->Update(delta);
}

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

  mData.spView->SetUDRenderFlags(UDRenderFlags::ClearTargets);

  mData.spSimpleCamera->SetPosition(Double3::create(0.5, -1.0, 0.5));
  mData.spSimpleCamera->SetSpeed(1.0);
  mData.spSimpleCamera->SetInvertedYAxis(true);
  mData.spSimpleCamera->SetPerspective(EP_PIf / 3.f);
  mData.spSimpleCamera->SetDepthPlanes(0.0001f, 7500.f);

  mData.spView->SetScene(mData.spScene);
  mData.spView->SetCamera(mData.spSimpleCamera);
  mData.spKernel->SetFocusView(mData.spView);

  mData.spView->Activate();

  ResourceManagerRef spResourceManager = mData.spKernel->GetResourceManager();

  if (spResourceManager)
  {
    Variant *pFile = data.getItem("--file");
    if (pFile)
      mData.filename = pFile->asString();

    // TODO: enable streamer once we have a tick running to update the streamer
    DataSourceRef spModelDS = spResourceManager->LoadResourcesFromFile({ { "src", mData.filename },{ "useStreamer", false } });
    if (spModelDS && spModelDS->GetNumResources() > 0)
    {
      mData.spUDModel = spModelDS->GetResourceAs<UDModel>(0);
    }

    mData.spUDNode->SetUDModel(mData.spUDModel);
    mData.spUDNode->SetPosition(Double3::create(0, 0, 0));

    Double4x4 modelMat = mData.spUDModel->GetUDMatrix();
    Double3 modelCorner = mData.spUDModel->GetUDMatrix().axis.t.toVector3();
    Double3 camPos = modelCorner + Double3{ modelMat.axis.x.x*2, modelMat.axis.y.y*2, modelMat.axis.z.z*2 };

    Double4x4 camMat = Double4x4::lookAt(camPos, modelCorner + Double3{ modelMat.axis.x.x / 2, modelMat.axis.y.y / 2, modelMat.axis.z.z / 2 });

    mData.spSimpleCamera->SetMatrix(camMat);
  }

  mData.spScene->GetRootNode()->AddChild(mData.spUDNode);

#if EP_DEBUG
    mData.spTestGeomNode = CreateTestModel(mData.spKernel);
    mData.spScene->GetRootNode()->AddChild(mData.spTestGeomNode);
#endif // EP_DEBUG

  mData.spUpdateSub = mData.spKernel->UpdatePulse.Subscribe(Delegate<void(double)>(&Update));

  mData.spScene->MakeDirty();
}

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
static void ViewerDeinit(String sender, String message, const Variant &data)
{
  mData.spUpdateSub->Unsubscribe();
  mData.spScene->GetRootNode()->RemoveChild(mData.spUDNode);
#if EP_DEBUG
  mData.spScene->GetRootNode()->RemoveChild(mData.spTestGeomNode);
#endif // EP_DEBUG
  if (mData.spCITimerSub)
    mData.spCITimerSub->Unsubscribe();

  mData.spUDNode = nullptr;
  mData.spUDModel = nullptr;
  mData.spView = nullptr;
  mData.spScene = nullptr;
  mData.spSimpleCamera = nullptr;
  mData.spTestGeomNode = nullptr;
  mData.spCITimerSub = nullptr;
  mData.spUpdateSub = nullptr;
  mData.spCITimer = nullptr;
}

// ---------------------------------------------------------------------------------------
// Author: David Ely, September 2015
int main(int argc, char* argv[])
{

#ifdef DINKEY_DONGLE_PROTECT
  // NOTE: The dinkey manual recommends this code be spread amongst other code
  // to make it harder for crackers to follow what is going on.
  // TODO: Do this.

  // Prepare the DRIS structure
  DRIS dris;
  dinkey_random_set(&dris, sizeof(DRIS)); // Init the DRIS structure with random values
  memcpy(dris.header, "DRIS", 4);
  dris.size = sizeof(DRIS);
  dris.function = PROTECTION_CHECK;
  dris.flags = 0;

  // Perform dongle check
  dinkey_CryptDRIS(&dris, dris.seed1, dris.seed2);			// encrypt DRIS (!!!!you should separate from DDProtCheck for greater security)
  int ret_code = DDProtCheck(&dris, NULL);
  dinkey_CryptDRIS(&dris, dris.seed1, dris.seed2);			// decrypt DRIS (!!!!you should separate from DDProtCheck for greater security)

  // Check for dongle error
  if (ret_code != 0)
  {
    const char *errmsg = dinkey_GetError(ret_code, dris.ext_err);
    DebugFormat("Dongle error ({0}): {1}\n", ret_code, errmsg);
    return ret_code;
  }

  // Check SDSN (all our dongles have the same SDSN)
  if (dris.sdsn != DINKEY_DONGLE_SDSN)
  {
    DebugFormat("Dongle error: Incorrect SDSN!");
    return 10001;
  }

  // Check product code
  if (strcmp(dris.prodcode, DINKEY_DONGLE_PRODCODE) != 0)
  {
    DebugFormat("Dongle error: Incorrect Product Code!");
    return 10002;
  }

#endif // DINKEY_DONGLE_PROTECT

  epInitMemoryTracking();
  if (argc > 1)
  {
    if (String(argv[1]).eqIC("shut_down_test"))
    {
      mData.shutdownTest = true;
      argc = 1;
    }
  }

  using namespace ep;
  mData.rendererThreadCount = epGetHardwareThreadCount() - 1;
  ProcessCmdline(argc, argv);

  Result result = Result::Success;
  try
  {
    mData.spKernel = SharedPtr<Kernel>(Kernel::CreateInstance(epParseCommandLine(argc, argv), mData.rendererThreadCount));

    mData.spKernel->RegisterMessageHandler("init", &ViewerInit);
    mData.spKernel->RegisterMessageHandler("deinit", &ViewerDeinit);

    if (mData.shutdownTest)
    {
      mData.spCITimer = mData.spKernel->CreateComponent<Timer>({ { "countdown", 4 } });
      mData.spCITimerSub = mData.spCITimer->Elapsed.Subscribe([]() { Kernel::GetInstance()->Quit(); });
    }

    mData.spKernel->RunMainLoop();

    mData.spKernel = nullptr;
  }
  catch (std::exception &e)
  {
    DebugFormat("Unhandled exception: {0}\n", e.what());
    result = Result::Failure;
  }
  catch (...)
  {
    epDebugWrite("Unhandled exception!\n");
    result = Result::Failure;
  }

  return result != Result::Success ? 1 : 0;
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


// This pushes all calls to operator new through our memory tracking.
#if EP_DEBUG

#if defined (new)
#undef new
#endif // defined (new)

#if defined (delete)
#undef delete
#endif // defined (new)


void* operator new(std::size_t sz)
{
  return epAlloc(sz);
}
void operator delete(void* ptr) noexcept
{
  epFree(ptr);
}

void* operator new[](std::size_t sz)
{
  return epAlloc(sz);
}

void operator delete[](void* ptr) noexcept
{
  epFree(ptr);
}
#endif //EP_DEBUG
