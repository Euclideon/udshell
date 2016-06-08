#include <QLoggingCategory>
#include <QQuickItem>

#include "ep/cpp/kernel.h"
#include "helpers.h"
#include "components/logger.h"
#include "ep/cpp/component/window.h"
#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/component/project.h"
#include "ep/cpp/component/activity.h"
#include "ep/cpp/component/resource/menu.h"
#include "ep/cpp/component/commandmanager.h"
#include "ep/cpp/component/resourcemanager.h"
#include "components/pluginmanager.h"
#include "components/file.h"
#include "messagebox.h"

#include "ep/cpp/component/primitivegenerator.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/resource/material.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/resource/shader.h"
#include "ep/cpp/component/timer.h"
#include "ep/cpp/component/scene.h"
#include "ep/cpp/component/resource/metadata.h"
#include "ep/cpp/component/node/geomnode.h"

using namespace ep;

static String appTitle = "Euclideon Platform";

static SharedPtr<Kernel> spKernel = nullptr;
static WindowRef spMainWindow;
static UIComponentRef spTopLevelUI;
static UIComponentRef spMessageBox;
static MenuRef spMenu;
static MenuRef spToolBar;
static ProjectRef spProject;
static TimerRef spCITimer;
static SubscriptionRef spCITimerSub;
static bool shutdownTest = false;
static GeomNodeRef spTestGeomNode;

MutableString256 projectName;

// ---------------------------------------------------------------------------------------
void DbgMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if (spKernel)
  {
    // TODO: replace this with something more robust - maybe a full logging system and status console
    switch (type) {
      case QtWarningMsg:
        spKernel->LogWarning(2, SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
      case QtCriticalMsg:
      case QtFatalMsg:
        spKernel->LogError(SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
      case QtDebugMsg:
      default:
        spKernel->LogDebug(2, SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
    }
  }
  else
  {
    epDebugPrintf("Qt Dbg: %s (%s:%d, %s)\n", msg.toUtf8().data(), context.file, context.line, context.function);
  }
}

MutableString<0> ReadResourceFile(String src)
{
  QFile file(QString(src.toStringz()));
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QByteArray qArray = file.readAll();
    file.close();

    return String(qArray.constData(), qArray.length());
  }

  return String();
}

MutableString256 GetNameFromFilePath(String path)
{
  if (path.exists('/'))
    path = path.getRightAtLast(String("/"), false);
  if (path.exists('.'))
    path = path.getLeftAtLast(String("."), false);

  return path;
}

void AddUIActivity(ActivityRef spActivity)
{
  spTopLevelUI->Call("addactivity", spActivity);
}

void RemoveUIActivity(ActivityRef spActivity)
{
  spTopLevelUI->Call("removeactivity", String(spActivity->uid));
}

void OnActivityChanged(String uid)
{
  auto spActiveActivity = spProject->GetActiveActivity();

  if (spActiveActivity)
  {
    spActiveActivity->Deactivate();
    spProject->SetActiveActivity(nullptr);
  }

  if (uid.empty())
    return;

  ActivityRef spActivity = component_cast<Activity>(spKernel->FindComponent(uid));
  if (!spActivity)
  {
    spKernel->LogError("Unable to activate Activity \"{0}\". Component does not exist", uid);
    return;
  }

  spProject->SetActiveActivity(spActivity);
  spActivity->Activate();
}

void InitProject()
{
  // Enable menu items
  spMenu->SetItemProperties("File/Save Project", { { "enabled", true } });
  spMenu->SetItemProperties("File/Save Project As...", { { "enabled", true } });
  spMenu->SetItemProperties("File/New.../New Activity", { { "enabled", true } });

  // Enable toolbar buttons
  spToolBar->SetItemProperties("Save Project", { { "enabled", true } });
  spToolBar->SetItemProperties("Save Project As...", { { "enabled", true } });
  spToolBar->SetItemProperties("New.../New Activity", { { "enabled", true } });

  // Enable shortcuts
  auto spCommandManager = spKernel->GetCommandManager();
  spCommandManager->EnableShortcut("saveproject");
  spCommandManager->EnableShortcut("saveprojectas");
  spCommandManager->EnableShortcut("newactivity");
}

void NewProject(String filePath)
{
  MutableString256 sFilePath = filePath;

  if (!filePath.endsWithIC(".epproj"))
    sFilePath.append(".epproj");

  if (spProject)
  {
    Array<ActivityRef> activities = spProject->GetActivities();
    for (size_t i = 0; i < activities.length; i++)
      RemoveUIActivity(activities[i]);

    spKernel->GetResourceManager()->ClearResources();
  }
  spProject = nullptr;

  spProject = spKernel->CreateComponent<Project>({ { "name", "project" } });
  spProject->SetSrc(filePath);

  projectName = GetNameFromFilePath(filePath);
  spMainWindow->Set("title", SharedString::format("{0} - {1}", projectName, appTitle));

  InitProject();
}

void OpenProject(String filePath)
{
  ProjectRef spNewProject;

  try
  {
    spNewProject = spKernel->CreateComponent<Project>({ { "src", filePath } });
  }
  catch (EPException &)
  {
    spKernel->LogWarning(2, "Couldn't open project file \"{0}\"", MutableString256().urlDecode(filePath));
    return;
  }

  if (spProject)
  {
    Array<ActivityRef> activities = spProject->GetActivities();
    for (size_t i = 0; i < activities.length; i++)
    {
      RemoveUIActivity(activities[i]);
      spProject->RemoveActivity(activities[i]);
    }

    spKernel->GetResourceManager()->ClearResources();
  }

  spProject = spNewProject;
  spProject->SetName("project");

  projectName = GetNameFromFilePath(filePath);
  spMainWindow->Set("title", SharedString::format("{0} - {1}", projectName, appTitle));

  // Load Activities from project file
  if (spProject)
  {
    auto spActivities = spProject->GetActivities();
    for (auto spActivity : spActivities)
      AddUIActivity(spActivity);
  }

  InitProject();
}

void SaveProject()
{
  if (spProject)
    spProject->SaveProject();
}

void NewActivity(String typeID)
{
  auto spActivity = component_cast<Activity>(spKernel->CreateComponent(typeID, nullptr));
  spProject->AddActivity(spActivity);
  AddUIActivity(spActivity);
}

void SaveProjectAs(String filePath)
{
  projectName = GetNameFromFilePath(filePath);
  spMainWindow->Set("title", SharedString::format("{0} - {1}", projectName, appTitle));

  spProject->SetSrc(filePath);
  spProject->SaveProject();
}

Array<Variant::VarMap> GetActivitiesInfo()
{
  Array<Variant::VarMap> infoArray;

  Array<const ComponentDesc *> descs = spKernel->GetDerivedComponentDescs<Activity>(false);
  for (auto desc : descs)
  {
    Variant::VarMap activityInfo;
    activityInfo.insert("namespace", desc->info.nameSpace);
//    activityInfo.insert("typename", desc->info.name);
    activityInfo.insert("identifier", desc->info.identifier);
    activityInfo.insert("name", desc->info.displayName);
    activityInfo.insert("description", desc->info.description);
    infoArray.pushBack(std::move(activityInfo));
  }

  return infoArray;
}

void Deinit(String sender, String message, const Variant &data)
{
  //SaveProject(); // Uncomment this if you want to dump the state of the Activities into a project file on program close

  spMainWindow = nullptr;
  spProject = nullptr;
  spTopLevelUI = nullptr;
  spMenu = nullptr;
  spToolBar = nullptr;
  spMessageBox = nullptr;

  spCITimerSub = nullptr;
  spCITimer = nullptr;

  spTestGeomNode = nullptr;
}

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

  // Generate Cube
  auto generator = kernel->CreateComponent<PrimitiveGenerator>();

  ArrayBufferRef vertexBuffer = kernel->CreateComponent<ArrayBuffer>();
  ArrayBufferRef indexBuffer = kernel->CreateComponent<ArrayBuffer>();

  generator->GenerateCube(vertexBuffer, indexBuffer);

  MetadataRef metadata = vertexBuffer->GetMetadata();
  metadata->Get("attributeinfo")[0].insertItem("name", "a_position");

  // Colour Buffer
  ArrayBufferRef colourBuffer = kernel->CreateComponent<ArrayBuffer>();
  {
    colourBuffer->AllocateFromData(Slice<const Float4>{
      Float4{ 1.0f, 1.0f, 1.0f, 1.0f },
      Float4{ 1.0f, 0.0f, 0.0f, 1.0f },
      Float4{ 0.0f, 1.0f, 0.0f, 1.0f },
      Float4{ 0.0f, 0.0f, 1.0f, 1.0f },

      Float4{ 1.0f, 1.0f, 0.0f, 1.0f },
      Float4{ 1.0f, 0.0f, 1.0f, 1.0f },
      Float4{ 0.0f, 1.0f, 1.0f, 1.0f },
      Float4{ 0.5f, 0.5f, 1.0f, 1.0f }
    });
    metadata = colourBuffer->GetMetadata();
    metadata->Get("attributeinfo")[0].insertItem("name", "a_color");
  }

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
  model->SetName("TestModel");
  model->AddVertexArray(vertexBuffer);
  model->AddVertexArray(colourBuffer);
  model->SetIndexArray(indexBuffer);
  model->SetMaterial(material);
  model->SetRenderList(RenderList { PrimType::Triangles, size_t(0), size_t(0), indexBuffer->GetLength() });

  GeomNodeRef geomNode = kernel->CreateComponent<GeomNode>();
  geomNode->SetModel(model);

  return geomNode;
}

void Register(String sender, String message, const Variant &data)
{
#if 0
  // NOTE: Use to test off disk
  spKernel->Call("registerqmlcomponents", "shell/qml");
#else
  spKernel->Call("registerqmlcomponents", ":/EP");
#endif
}

void Init(String sender, String message, const Variant &data)
{
  epscope(fail) { if (!spMainWindow) spKernel->LogError("Error creating MainWindow UI Component\n"); };
  spMainWindow = component_cast<Window>(spKernel->CreateComponent("ui.appwindow"));

  epscope(fail) { if (!spTopLevelUI) spKernel->LogError("Error creating top Level UI Component\n"); };
  spTopLevelUI = component_cast<UIComponent>(spKernel->CreateComponent("ui.main"));

  epscope(fail) { if (!spMessageBox) spKernel->LogError("Error creating MessageBox UI Component\n"); };
  spMessageBox = component_cast<UIComponent>(spKernel->CreateComponent("ui.messagebox", Variant::VarMap{ { "name", "messagebox" } }));
  spTopLevelUI->Set("messageboxcomp", spMessageBox);

  UIComponentRef spConsole;
  epscope(fail) { if (!spConsole) spKernel->LogError("Error creating Console UI Component\n"); };
  spConsole = component_cast<UIComponent>(spKernel->CreateComponent("ui.console"));
  spTopLevelUI->Set("uiconsole", spConsole);

  // Load menus
  String menusPath(":/menus.xml");
  MutableString<0> menuStr = ReadResourceFile(menusPath);
  if(menuStr.empty())
    spKernel->LogWarning(2, "Menus XML file \"{0}\" does not exist.", menusPath);

  spMenu = spKernel->CreateComponent<Menu>({ { "src", menuStr } });
  spTopLevelUI->Set("menucomp", spMenu);

  // Load toolbar
  String toolBarPath(":/toolbar.xml");
  MutableString<0> toolBarStr = ReadResourceFile(toolBarPath);
  if (toolBarStr.empty())
    spKernel->LogWarning(2, "Toolbar XML file \"{0}\" does not exist.", toolBarPath);

  spToolBar = spKernel->CreateComponent<Menu>({ { "src", toolBarStr } });
  spTopLevelUI->Set("toolbarcomp", spToolBar);

  // New Activity selector panel
  auto spActivitySelector = component_cast<UIComponent>(spKernel->CreateComponent("ui.activityselector"));
  spActivitySelector->Set("activitiesinfo", GetActivitiesInfo());
  spTopLevelUI->Set("activityselector", spActivitySelector);

  // Subscribe to UI events
  spTopLevelUI->Subscribe("newprojectsignal", Delegate<void(String)>(&NewProject));
  spTopLevelUI->Subscribe("openprojectsignal", Delegate<void(String)>(&OpenProject));
  spTopLevelUI->Subscribe("saveprojectsignal", Delegate<void(void)>(&SaveProject));
  spTopLevelUI->Subscribe("saveprojectassignal", Delegate<void(String)>(&SaveProjectAs));
  spTopLevelUI->Subscribe("newactivitysignal", Delegate<void(String)>(&NewActivity));
  spTopLevelUI->Subscribe("activitychanged", Delegate<void(String)>(&OnActivityChanged));

  spMainWindow->SetTopLevelUI(spTopLevelUI);

#ifdef _DEBUG
  OpenProject("testproj.epproj");
#endif

  if (shutdownTest)
  {
    spCITimer = spKernel->CreateComponent<Timer>({ { "duration", 4 * 1000 }, { "timertype", "CountDown" } });
    spCITimerSub = spCITimer->Elapsed.Subscribe([]() { Kernel::GetInstance()->Quit(); });
  }

  spTestGeomNode = CreateTestModel(spKernel);

  SceneRef spDircubeScene;
  {
    ComponentRef spComp = spKernel->FindComponent("DirCubeScene");
    if (spComp)
    {
      spDircubeScene = component_cast<Scene>(spComp);
      NodeRef rootNode = spDircubeScene->GetRootNode();
      if (rootNode)
      {
        rootNode->AddChild(spTestGeomNode);
      }
    }
  }
}

// ---------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  epInitMemoryTracking();
  if (argc > 1)
  {
    if (String(argv[1]).eqIC("shut_down_test"))
      shutdownTest = true;
  }

  // install our qt message handler
  qInstallMessageHandler(DbgMessageHandler);
  //QLoggingCategory::setFilterRules("qt.*=true");

  // TEMP: this is just for testing - force qt to use the threaded renderer on windows
  // 5.5 uses this by default
  qputenv("QSG_RENDER_LOOP", "threaded");

  try
  {
    // create a kernel
    int threadCount = epGetHardwareThreadCount() - 1;
    spKernel = SharedPtr<Kernel>(Kernel::CreateInstance(epParseCommandLine(argc, argv), threadCount));

    spKernel->RegisterMessageHandler("register", &Register);
    spKernel->RegisterMessageHandler("init", &Init);
    spKernel->RegisterMessageHandler("deinit", &Deinit);

    spKernel->RunMainLoop();

    spKernel = nullptr;
  }
  catch (std::exception &e)
  {
    epDebugFormat("Unhandled exception: {0}\n", e.what());
    return 1;
  }
  catch (...)
  {
    epDebugWrite("Unhandled exception!\n");
    return 1;
  }

  return 0;
}

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
