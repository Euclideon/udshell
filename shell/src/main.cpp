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
#include "ep/cpp/component/file.h"
#include "components/pluginmanager.h"
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
#include "dinkey.h"

using namespace ep;

static String appTitle = "Euclideon Platform";

static SharedPtr<Kernel> spKernel = nullptr;
static WindowRef spMainWindow;
static UIComponentRef spTopLevelUI;
static UIComponentRef spMessageBox;
static MenuRef spMenu;
//static MenuRef spToolBar;
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
        spKernel->logWarning(2, SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
      case QtCriticalMsg:
      case QtFatalMsg:
        spKernel->logError(SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
      case QtDebugMsg:
      default:
        spKernel->logDebug(2, SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
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
  spTopLevelUI->call("addActivity", spActivity);
}

void RemoveUIActivity(ActivityRef spActivity)
{
  spTopLevelUI->call("removeActivity", String(spActivity->uid));
}

void onActivityChanged(String uid)
{
  auto spActiveActivity = spProject->getActiveActivity();

  if (spActiveActivity)
  {
    spActiveActivity->deactivate();
    spProject->setActiveActivity(nullptr);
  }

  if (uid.empty())
    return;

  ActivityRef spActivity = component_cast<Activity>(spKernel->findComponent(uid));
  if (!spActivity)
  {
    spKernel->logError("Unable to activate Activity \"{0}\". Component does not exist", uid);
    return;
  }

  spProject->setActiveActivity(spActivity);
  spActivity->activate();
}

void InitProject()
{
  // Enable menu items
  spMenu->setItemProperties("File/Save Project", { { "enabled", true } });
  spMenu->setItemProperties("File/Save Project As...", { { "enabled", true } });
  spMenu->setItemProperties("File/New.../New Activity", { { "enabled", true } });

  // Enable toolbar buttons
  //spToolBar->setItemProperties("Save Project", { { "enabled", true } });
  //spToolBar->setItemProperties("Save Project As...", { { "enabled", true } });
  //spToolBar->setItemProperties("New.../New Activity", { { "enabled", true } });

  // Enable shortcuts
  auto spCommandManager = spKernel->getCommandManager();
  spCommandManager->enableShortcut("saveProject");
  spCommandManager->enableShortcut("saveProjectAs");
  spCommandManager->enableShortcut("newActivity");
}

void newProject(String filePath)
{
  MutableString256 sFilePath = filePath;

  if (!filePath.endsWithIC(".epproj"))
    sFilePath.append(".epproj");

  if (spProject)
  {
    Array<ActivityRef> activities = spProject->getActivities();
    for (size_t i = 0; i < activities.length; i++)
      RemoveUIActivity(activities[i]);

    spKernel->getResourceManager()->clearResources();
  }
  spProject = nullptr;

  spProject = spKernel->createComponent<Project>({ { "name", "project" } });
  spProject->setSrc(sFilePath);

  projectName = GetNameFromFilePath(sFilePath);
  spMainWindow->set("title", SharedString::format("{0} - {1}", projectName, appTitle));

  InitProject();
}

void openProject(String filePath)
{
  ProjectRef spNewProject;

  try
  {
    spNewProject = spKernel->createComponent<Project>({ { "src", filePath } });
  }
  catch (EPException &)
  {
    spKernel->logWarning(2, "Couldn't open project file \"{0}\"", MutableString256().urlDecode(filePath));
    return;
  }

  if (spProject)
  {
    Array<ActivityRef> activities = spProject->getActivities();
    for (size_t i = 0; i < activities.length; i++)
    {
      RemoveUIActivity(activities[i]);
      spProject->removeActivity(activities[i]);
    }

    spKernel->getResourceManager()->clearResources();
  }

  spProject = spNewProject;
  spProject->setName("project");

  projectName = GetNameFromFilePath(filePath);
  spMainWindow->set("title", SharedString::format("{0} - {1}", projectName, appTitle));

  // Load Activities from project file
  if (spProject)
  {
    auto spActivities = spProject->getActivities();
    for (auto spActivity : spActivities)
      AddUIActivity(spActivity);
  }

  InitProject();
}

void saveProject()
{
  if (spProject)
    spProject->saveProject();
}

void newActivity(String typeID)
{
  auto spActivity = component_cast<Activity>(spKernel->createComponent(typeID, nullptr));
  spProject->addActivity(spActivity);
  AddUIActivity(spActivity);
}

void saveProjectAs(String filePath)
{
  projectName = GetNameFromFilePath(filePath);
  spMainWindow->set("title", SharedString::format("{0} - {1}", projectName, appTitle));

  spProject->setSrc(filePath);
  spProject->saveProject();
}

Array<Variant::VarMap> GetActivitiesInfo()
{
  Array<Variant::VarMap> infoArray;

  Array<const ComponentDesc *> descs = spKernel->getDerivedComponentDescs<Activity>(false);
  for (auto desc : descs)
  {
    Variant::VarMap::MapType activityInfo;
    activityInfo.insert("namespace", desc->info.nameSpace);
    activityInfo.insert("typename", desc->info.name);
    activityInfo.insert("identifier", desc->info.identifier);
//    activityInfo.insert("name", desc->info.displayName); // TODO: add this back at some point?
    activityInfo.insert("name", desc->info.name);
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
  //spToolBar = nullptr;
  spMessageBox = nullptr;

  spCITimerSub = nullptr;
  spCITimer = nullptr;

  spTestGeomNode = nullptr;
}

#if EP_DEBUG
static GeomNodeRef CreateTestModel(KernelRef kernel)
{
  // Vertex Shader
  ShaderRef vertexShader = kernel->createComponent<Shader>();
  {
    vertexShader->setType(ShaderType::VertexShader);

    const char shaderText[] = "attribute vec3 a_position;\n"
                              "attribute vec4 a_color;\n"
                              "varying vec4 v_color;\n"
                              "uniform mat4 u_mfwvp;\n"
                              "void main()\n"
                              "{\n"
                              "  v_color = a_color;\n"
                              "  gl_Position = u_mfwvp * vec4(a_position, 1.0);\n"
                              "}\n";
    vertexShader->setCode(shaderText);
  }

  // Pixel Shader
  ShaderRef pixelShader = kernel->createComponent<Shader>();
  {
    pixelShader->setType(ShaderType::PixelShader);

    const char shaderText[] = "varying vec4 v_color;\n"
                              "void main()\n"
                              "{\n"
                              "  gl_FragColor = v_color;\n"
                              "}\n";
    pixelShader->setCode(shaderText);
  }

  // Material
  MaterialRef material = kernel->createComponent<Material>();
  material->setShader(ShaderType::VertexShader, vertexShader);
  material->setShader(ShaderType::PixelShader, pixelShader);
#if 0
  material->SetTexture(0, texture);
#endif // 0

  ArrayBufferRef vertexBuffer = kernel->createComponent<ArrayBuffer>();
  ArrayBufferRef indexBuffer = kernel->createComponent<ArrayBuffer>();

  // Generate Cube
  PrimitiveGenerator::generateCube(vertexBuffer, indexBuffer);

  MetadataRef metadata = vertexBuffer->getMetadata();
  metadata->insertAt("a_position", "attributeInfo", 0, "name");

  // Colour Buffer
  ArrayBufferRef colourBuffer = kernel->createComponent<ArrayBuffer>();
  {
    colourBuffer->allocateFromData(Slice<const Float4>{
      Float4{ 1.0f, 1.0f, 1.0f, 1.0f },
      Float4{ 1.0f, 0.0f, 0.0f, 1.0f },
      Float4{ 0.0f, 1.0f, 0.0f, 1.0f },
      Float4{ 0.0f, 0.0f, 1.0f, 1.0f },

      Float4{ 1.0f, 1.0f, 0.0f, 1.0f },
      Float4{ 1.0f, 0.0f, 1.0f, 1.0f },
      Float4{ 0.0f, 1.0f, 1.0f, 1.0f },
      Float4{ 0.5f, 0.5f, 1.0f, 1.0f }
    });
    metadata = colourBuffer->getMetadata();
    metadata->insertAt("a_color", "attributeInfo", 0, "name");
  }

#if 0
  // Texture
  ArrayBufferRef texture = kernel->createComponent<ArrayBuffer>();
  {
    texture->Allocate("White", sizeof(uint32_t), { 32, 32 });
    Slice<void> data  = texture->Map();
    memset(data.ptr, 0xFF, data.length);
    texture->Unmap();
  }
#endif // 0

  ModelRef model = kernel->createComponent<Model>();
  model->setName("TestModel");
  model->addVertexArray(vertexBuffer);
  model->addVertexArray(colourBuffer);
  model->setIndexArray(indexBuffer);
  model->setMaterial(material);
  model->setRenderList(RenderList { PrimType::Triangles, size_t(0), size_t(0), indexBuffer->getLength() });

  GeomNodeRef geomNode = kernel->createComponent<GeomNode>();
  geomNode->setModel(model);

  return geomNode;
}
#endif // EP_DEBUG

void Register(String sender, String message, const Variant &data)
{
#if 0
  // NOTE: Use to test off disk
  spKernel->call("registerQmlComponents", "shell/qml");
#else
  spKernel->call("registerQmlComponents", ":/EP");
#endif
}

void Init(String sender, String message, const Variant &data)
{
  epscope(fail) { if (!spMainWindow) spKernel->logError("Error creating MainWindow UI Component\n"); };
  spMainWindow = component_cast<Window>(spKernel->createComponent("ui.AppWindow"));

  epscope(fail) { if (!spTopLevelUI) spKernel->logError("Error creating top Level UI Component\n"); };
  spTopLevelUI = component_cast<UIComponent>(spKernel->createComponent("ui.Main"));

  epscope(fail) { if (!spMessageBox) spKernel->logError("Error creating MessageBox UI Component\n"); };
  spMessageBox = component_cast<UIComponent>(spKernel->createComponent("ui.MessageBox", Variant::VarMap{ { "name", "messagebox" } }));
  spTopLevelUI->set("messageBox", spMessageBox);

  UIComponentRef spConsole;
  epscope(fail) { if (!spConsole) spKernel->logError("Error creating Console UI Component\n"); };
  spConsole = component_cast<UIComponent>(spKernel->createComponent("ui.Console"));
  spTopLevelUI->set("uiConsole", spConsole);

  // Load menus
  String menusPath(":/menus.xml");
  MutableString<0> menuStr = ReadResourceFile(menusPath);
  if(menuStr.empty())
    spKernel->logWarning(2, "Menus XML file \"{0}\" does not exist.", menusPath);

  spMenu = spKernel->createComponent<Menu>({ { "src", menuStr } });
  spTopLevelUI->set("menu", spMenu);

  // Load toolbar
  /*String toolBarPath(":/toolbar.xml");
  MutableString<0> toolBarStr = ReadResourceFile(toolBarPath);
  if (toolBarStr.empty())
    spKernel->logWarning(2, "Toolbar XML file \"{0}\" does not exist.", toolBarPath);

  spToolBar = spKernel->createComponent<Menu>({ { "src", toolBarStr } });
  spTopLevelUI->set("toolbarcomp", spToolBar);*/

  // New Activity selector panel
  auto spActivitySelector = component_cast<UIComponent>(spKernel->createComponent("ui.ActivitySelector"));
  spActivitySelector->set("activitiesInfo", GetActivitiesInfo());
  spTopLevelUI->set("activitySelector", spActivitySelector);

  // Subscribe to UI events
  spTopLevelUI->subscribe("newProjectSelected", Delegate<void(String)>(&newProject));
  spTopLevelUI->subscribe("openProjectSelected", Delegate<void(String)>(&openProject));
  spTopLevelUI->subscribe("saveProjectSelected", Delegate<void(void)>(&saveProject));
  spTopLevelUI->subscribe("saveProjectAsSelected", Delegate<void(String)>(&saveProjectAs));
  spTopLevelUI->subscribe("newActivitySelected", Delegate<void(String)>(&newActivity));
  spTopLevelUI->subscribe("activityChanged", Delegate<void(String)>(&onActivityChanged));

  spMainWindow->setTopLevelUI(spTopLevelUI);

  Variant *pProject = data.getItem("--project");
  if (!pProject)
    pProject = data.getItem("-p");
  if (pProject)
  {
    openProject(pProject->asString());
  }
  else
  {
#ifdef _DEBUG
    openProject("testproj.epproj");
#endif
  }

  if (shutdownTest)
  {
    spCITimer = spKernel->createComponent<Timer>({ { "countdown", 4 } });
    spCITimerSub = spCITimer->elapsed.subscribe([]() { Kernel::getInstance()->quit(); });
  }

#if EP_DEBUG
  spTestGeomNode = CreateTestModel(spKernel);
#endif // EP_DEBUG
  SceneRef spDircubeScene;
  {
    ComponentRef spComp = spKernel->findComponent("DirCubeScene");
    if (spComp)
    {
      spDircubeScene = component_cast<Scene>(spComp);
      NodeRef rootNode = spDircubeScene->getRootNode();
      if (rootNode)
      {
        rootNode->addChild(spTestGeomNode);
      }
    }
  }
}

// ---------------------------------------------------------------------------------------
int main(int argc, char *argv[])
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
      shutdownTest = true;
  }

  // install our qt message handler
  qInstallMessageHandler(DbgMessageHandler);
  //QLoggingCategory::setFilterRules("qt.*=true");

  try
  {
    // create a kernel
    int threadCount = getHardwareThreadCount() - 1;
    spKernel = SharedPtr<Kernel>(Kernel::createInstance(parseCommandLine(argc, argv), threadCount));

    spKernel->registerMessageHandler("register", &Register);
    spKernel->registerMessageHandler("init", &Init);
    spKernel->registerMessageHandler("deinit", &Deinit);

    spKernel->runMainLoop();

    spKernel = nullptr;
  }
  catch (std::exception &e)
  {
    DebugFormat("Unhandled exception: {0}\n", e.what());
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
