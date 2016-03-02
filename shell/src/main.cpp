#include <QLoggingCategory>
#include <QQuickItem>
#include "driver/qt/util/typeconvert_qt.h"

#include "ep/cpp/kernel.h"
#include "helpers.h"
#include "components/logger.h"
#include "ep/cpp/component/window.h"
#include "ep/cpp/component/uicomponent.h"
#include "components/uiconsole.h"
#include "components/project.h"
#include "ep/cpp/component/activity.h"
#include "ep/cpp/component/resource/menu.h"
#include "ep/cpp/component/commandmanager.h"
#include "ep/cpp/component/resourcemanager.h"
#include "components/pluginmanager.h"
#include "components/file.h"
#include "hal/debugfont.h"
#include "messagebox.h"
#include "components/timer.h"

using namespace ep;

static String appTitle = "Euclideon Platform";
static QString defaultTheme = "qrc:/qml/themes/shelltheme.qml";

static SharedPtr<Kernel> spKernel = nullptr;
static WindowRef spMainWindow;
static UIComponentRef spTopLevelUI;
static UIComponentRef spMessageBox;
static MenuRef spMenu;
static MenuRef spToolBar;
static ProjectRef spProject;
static TimerRef spCITimer;
static SubscriptionRef spCITimerSub;
static bool CITest = false;

MutableString256 projFilePath;
MutableString256 projectName;

// ---------------------------------------------------------------------------------------
void DbgMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if (spKernel)
  {
    // TODO: replace this with something more robust - maybe a full logging system and status console
    switch (type) {
      case QtDebugMsg:
        spKernel->LogDebug(2, SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
      case QtWarningMsg:
        spKernel->LogWarning(2, SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
      case QtCriticalMsg:
      case QtFatalMsg:
        spKernel->LogError(SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
    }
  }
  else
  {
    epDebugPrintf("Qt Dbg: %s (%s:%d, %s)\n", msg.toUtf8().data(), context.file, context.line, context.function);
  }
}

void RegisterEPControls()
{
  // Register Theme
  EPTHROW_IF(qmlRegisterSingletonType(QUrl(defaultTheme), "epThemes", 0, 1, "Theme") == -1, epR_Failure, "qmlRegisterSingletonType \"epThemes\" Failed");

  EPTHROW_IF(qmlRegisterType(QUrl("qrc:/qml/epcontrols/epmenubar.qml"), "epControls", 0, 1, "EPMenuBar") == -1, epR_Failure, "qmlRegisterType \"EPMenuBar\" Failed");
  EPTHROW_IF(qmlRegisterType(QUrl("qrc:/qml/epcontrols/epmenu.qml"), "epControls", 0, 1, "EPMenu") == -1, epR_Failure, "qmlRegisterType \"EPMenu\" Failed");
  EPTHROW_IF(qmlRegisterType(QUrl("qrc:/qml/epcontrols/eptoolbar.qml"), "epControls", 0, 1, "EPToolBar") == -1, epR_Failure, "qmlRegisterType \"EPToolBar\" Failed");
  EPTHROW_IF(qmlRegisterType(QUrl("qrc:/qml/epcontrols/epmenubutton.qml"), "epControls", 0, 1, "EPMenuButton") == -1, epR_Failure, "qmlRegisterType \"EPMenuButton\" Failed");
  EPTHROW_IF(qmlRegisterType(QUrl("qrc:/qml/epcontrols/eptoolbutton.qml"), "epControls", 0, 1, "EPToolButton") == -1, epR_Failure, "qmlRegisterType \"EPToolButton\" Failed");
  EPTHROW_IF(qmlRegisterType(QUrl("qrc:/qml/epcontrols/epfiledialog.qml"), "epControls", 0, 1, "EPFileDialog") == -1, epR_Failure, "qmlRegisterType \"EPFileDialog\" Failed");
  EPTHROW_IF(qmlRegisterType(QUrl("qrc:/qml/epcontrols/eptoolpanelmanager.qml"), "epControls", 0, 1, "EPToolPanelManager") == -1, epR_Failure, "qmlRegisterType \"EPToolPanelManager\" Failed");
  EPTHROW_IF(qmlRegisterType(QUrl("qrc:/qml/epcontrols/eplistmodel.qml"), "epControls", 0, 1, "EPListModel") == -1, epR_Failure, "qmlRegisterType \"EPListModel\" Failed");
  EPTHROW_IF(qmlRegisterType(QUrl("qrc:/qml/epcontrols/eplistview.qml"), "epControls", 0, 1, "EPListView") == -1, epR_Failure, "qmlRegisterType \"EPListView\" Failed");
  EPTHROW_IF(qmlRegisterType(QUrl("qrc:/qml/epcontrols/eptableview.qml"), "epControls", 0, 1, "EPTableView") == -1, epR_Failure, "qmlRegisterType \"EPTableView\" Failed");
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
    ClearError();
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
    activityInfo.Insert("id", desc->info.id);
    activityInfo.Insert("displayName", desc->info.displayName);
    activityInfo.Insert("description", desc->info.description);
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
}

void Init(String sender, String message, const Variant &data)
{
  RegisterEPControls();

  epscope(fail) { if (!spMainWindow) spKernel->LogError("Error creating MainWindow UI Component\n"); };
  spMainWindow = spKernel->Call("createqmlcomponent", "window", "qrc:/qml/window.qml", nullptr).as<WindowRef>();

  epscope(fail) { if (!spTopLevelUI) spKernel->LogError("Error creating top Level UI Component\n"); };
  spTopLevelUI = spKernel->Call("createqmlcomponent", "uicomponent", "qrc:/qml/main.qml", nullptr).as<UIComponentRef>();


  epscope(fail) { if (!spMessageBox) spKernel->LogError("Error creating MessageBox UI Component\n"); };
  spMessageBox = spKernel->Call("createqmlcomponent", "uicomponent", "qrc:/qml/components/messagebox.qml", Variant::VarMap{{ "name", "messagebox" }}).as<UIComponentRef>();
  spTopLevelUI->Set("messageboxcomp", spMessageBox);

  UIConsoleRef spConsole;
  epscope(fail) { if (!spConsole) spKernel->LogError("Error creating top Level UI Component\n"); };
  spConsole = spKernel->Call("createqmlcomponent", "uiconsole", "qrc:/kernel/console.qml", nullptr).as<UIConsoleRef>();
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

  // New Activity selecter panel
  auto spActivitySelecter = spKernel->Call("createqmlcomponent", "uicomponent", "qrc:/qml/components/activityselecter.qml", nullptr).as<UIComponentRef>();
  spActivitySelecter->Set("activitiesinfo", GetActivitiesInfo());
  spTopLevelUI->Set("activityselecter", spActivitySelecter);

  // TODO: Remove this once UIComponent cleans up its events
  if (!CITest)
  {
    // Subscribe to UI events
    spTopLevelUI->Subscribe("newprojectsignal", Delegate<void(String)>(&NewProject));
    spTopLevelUI->Subscribe("openprojectsignal", Delegate<void(String)>(&OpenProject));
    spTopLevelUI->Subscribe("saveprojectsignal", Delegate<void(void)>(&SaveProject));
    spTopLevelUI->Subscribe("saveprojectassignal", Delegate<void(String)>(&SaveProjectAs));
    spTopLevelUI->Subscribe("newactivitysignal", Delegate<void(String)>(&NewActivity));
    spTopLevelUI->Subscribe("activitychanged", Delegate<void(String)>(&OnActivityChanged));
  }

  spMainWindow->SetTopLevelUI(spTopLevelUI);

  if (!projFilePath.empty())
    OpenProject(projFilePath);
#ifdef _DEBUG
  else
    OpenProject("testproj.epproj");
#endif

  if (CITest)
  {
    spCITimer = spKernel->CreateComponent<Timer>({ { "duration", 4 * 1000 }, { "timertype", "CountDown" } });
    spCITimerSub = spCITimer->Elapsed.Subscribe([]() { Kernel::GetInstance()->Quit(); });
  }
}

// ---------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  epInitMemoryTracking();
  if (argc > 1)
  {
    if (String(argv[1]).eqIC("CITest"))
    {
      CITest = true;
    }
    else
    {
      projFilePath.append(argv[1]);
    }
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
