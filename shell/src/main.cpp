#include <QLoggingCategory>
#include <QQuickItem>
#include "driver/qt/util/typeconvert_qt.h"

#include "kernel.h"
#include "components/logger.h"
#include "components/window.h"
#include "ep/cpp/component/uicomponent.h"
#include "components/uiconsole.h"
#include "components/project.h"
#include "components/activities/viewer.h"
#include "components/resources/menu.h"
#include "components/commandmanager.h"
#include "components/file.h"
#include "hal/debugfont.h"
#include "messagebox.h"

using namespace ep;

#define APP_TITLE "Euclideon Platform"

static kernel::Kernel *pKernel = nullptr;
static WindowRef spMainWindow;
static UIComponentRef spTopLevelUI;
static UIComponentRef spMessageBox;
static MenuRef spMenu;
static MenuRef spToolBar;
static ProjectRef spProject;

MutableString256 projFilePath;
MutableString256 projectName;

// ---------------------------------------------------------------------------------------
void DbgMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if (pKernel)
  {
    // TODO: replace this with something more robust - maybe a full logging system and status console
    switch (type) {
      case QtDebugMsg:
        pKernel->LogDebug(2, SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
      case QtWarningMsg:
        pKernel->LogWarning(2, SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
        break;
      case QtCriticalMsg:
      case QtFatalMsg:
        pKernel->LogError(SharedString::format("Qt: {0} ({1}:{2}, {3})", msg.toUtf8().data(), context.file, context.line, context.function));
    }
  }
  else
  {
    udDebugPrintf("Qt Dbg: %s (%s:%d, %s)\n", msg.toUtf8().data(), context.file, context.line, context.function);
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
  auto spUI = spActivity->GetUI();

  MutableString<128> title;
  title.append(spActivity->uid);
  title[0] = toUpper(title[0]);

  spTopLevelUI->CallMethod("addactivity", String(spActivity->uid), String(title), spUI);
}

void RemoveUIActivity(ActivityRef spActivity)
{
  spTopLevelUI->CallMethod("removeactivity", String(spActivity->uid));
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

  ActivityRef spActivity = pKernel->FindComponent(uid);
  if (!spActivity)
  {
    pKernel->LogError("Unable to activate Activity \"{0}\". Component does not exist", uid);
    return;
  }

  ViewerRef spViewer = component_cast<Viewer>(spActivity);
  if (spViewer)
  {
    spTopLevelUI->SetProperty("simplecamera", spViewer->GetSimpleCamera());
    spTopLevelUI->SetProperty("view", spViewer->GetView());
  }

  spProject->SetActiveActivity(spActivity);
  spActivity->Activate();
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
  }
  spProject = nullptr;

  spProject = pKernel->CreateComponent<Project>({ { "name", "project" } });
  spProject->SetSrc(filePath);

  projectName = GetNameFromFilePath(filePath);
  spMainWindow->SetProperty("title", SharedString::format("{0} - {1}", projectName, APP_TITLE));

  spMenu->SetItemProperties("File/Save Project", { { "enabled", true } });
  spMenu->SetItemProperties("File/Save Project As...", { { "enabled", true } });
  spToolBar->SetItemProperties("Save Project", { { "enabled", true } });
  spToolBar->SetItemProperties("Save Project As...", { { "enabled", true } });

  spMessageBox->CallMethod("show", Variant::VarMap{
    { "title", "Some title" },
    { "text", "Some text" },
    { "iconType", Variant(MBIconType::Warning) }
  });
}

void OpenProject(String filePath)
{
  ProjectRef spNewProject = pKernel->CreateComponent<Project>({ { "src", filePath }, { "name", "project" } });
  if (!spNewProject)
  {
    pKernel->LogWarning(1, "Couldn't open project file \"{0}\"", MutableString256().urlDecode(filePath));
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
  }

  spProject = spNewProject;

  projectName = GetNameFromFilePath(filePath);
  spMainWindow->SetProperty("title", SharedString::format("{0} - {1}", projectName, APP_TITLE));

  spMenu->SetItemProperties("File/Save Project", { { "enabled", true } });
  spMenu->SetItemProperties("File/Save Project As...", { { "enabled", true } });
  spToolBar->SetItemProperties("Save Project", { { "enabled", true } });
  spToolBar->SetItemProperties("Save Project As...", { { "enabled", true } });

  // Load Activities from project file
  if (spProject)
  {
    auto spActivities = spProject->GetActivities();
    for (auto spActivity : spActivities)
      AddUIActivity(spActivity);
  }
}

void SaveProject()
{
  if (spProject)
    spProject->SaveProject();
}

void SaveProjectAs(String filePath)
{
  projectName = GetNameFromFilePath(filePath);
  spMainWindow->SetProperty("title", SharedString::format("{0} - {1}", projectName, APP_TITLE));

  spProject->SetSrc(filePath);
  spProject->SaveProject();
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
}

void Init(String sender, String message, const Variant &data)
{
  spMainWindow = pKernel->CreateComponent<Window>({ { "file", "qrc:/qml/window.qml" } });
  if (!spMainWindow)
  {
    pKernel->LogError("Error creating MainWindow UI Component\n");
    return;
  }

  spTopLevelUI = pKernel->CreateComponent<UIComponent>({ { "file", "qrc:/qml/main.qml" } });
  if (!spTopLevelUI)
  {
    pKernel->LogError("Error creating top Level UI Component\n");
    return;
  }

  spMessageBox = pKernel->CreateComponent<UIComponent>({ { "name", "messagebox" }, { "file", "qrc:/qml/messagebox.qml" } });
  if (!spMessageBox)
  {
    pKernel->LogError("Error creating MessageBox UI Component\n");
    return;
  }
  spTopLevelUI->SetProperty("messageboxcomp", spMessageBox);

  auto spConsole = pKernel->CreateComponent<UIConsole>({ { "file", "qrc:/kernel/console.qml" } });
  if (!spConsole)
  {
    pKernel->LogError("Error creating top Level UI Component\n");
    return;
  }
  spTopLevelUI->SetProperty("uiconsole", spConsole);

  // Load menus
  String menusPath(":/menus.xml");
  MutableString<0> menuStr = ReadResourceFile(menusPath);
  if(menuStr.empty())
    pKernel->LogWarning(2, "Menus XML file \"{0}\" does not exist.", menusPath);

  spMenu = pKernel->CreateComponent<Menu>({ { "src", menuStr } });
  if (spMenu)
    spTopLevelUI->SetProperty("menucomp", spMenu);

  // Load toolbar
  String toolBarPath(":/toolbar.xml");
  MutableString<0> toolBarStr = ReadResourceFile(toolBarPath);
  if (toolBarStr.empty())
    pKernel->LogWarning(2, "Toolbar XML file \"{0}\" does not exist.", toolBarPath);

  spToolBar = pKernel->CreateComponent<Menu>({ { "src", toolBarStr } });
  if (spToolBar)
    spTopLevelUI->SetProperty("toolbarcomp", spToolBar);

  // Subscribe to UI events
  spTopLevelUI->Subscribe("newprojectsignal", Delegate<void(String)>(&NewProject));
  spTopLevelUI->Subscribe("openprojectsignal", Delegate<void(String)>(&OpenProject));
  spTopLevelUI->Subscribe("saveprojectsignal", Delegate<void(void)>(&SaveProject));
  spTopLevelUI->Subscribe("saveprojectassignal", Delegate<void(String)>(&SaveProjectAs));
  spTopLevelUI->Subscribe("activitychanged", Delegate<void(String)>(&OnActivityChanged));


  spMainWindow->SetTopLevelUI(spTopLevelUI);

  if (!projFilePath.empty())
    OpenProject(projFilePath);
#ifdef _DEBUG
  else
    OpenProject("testproj.epproj");
#endif

  /*
  // Hardcode an example Activity in for testing purposes
  Array<KeyValuePair> cameraParams{
    { "speed", 1.0 },
    { "invertyaxis", true },
    { "perspective", 1.0471975512 },
    { "depthplanes", Slice<const double>({ 0.0001, 7500.0 }) }
  };

#if defined(EP_WINDOWS)
  auto spExampleActivity = pKernel->CreateComponent<Viewer>({ { "model", "v:/RnD/uds/Peterskirche(SolidScan).uds" },{ "camera", cameraParams } });
#else
  auto spExampleActivity = pKernel->CreateComponent<Viewer>({ { "model", "data/DirCube.upc" },{ "camera", cameraParams } });
#endif // EP_WINDOWS
  if (spExampleActivity)
  {
    AddUIActivity(spExampleActivity);
    spProject->AddActivity(spExampleActivity);
  }
  else
    pKernel->LogError("Error creating Viewer activity\n");
  */
}

// ---------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  udMemoryDebugTrackingInit();

  if (argc > 1)
    projFilePath.append(argv[1]);

  // install our qt message handler
  qInstallMessageHandler(DbgMessageHandler);
  //QLoggingCategory::setFilterRules("qt.*=true");

  // TEMP: this is just for testing - force qt to use the threaded renderer on windows
  // 5.5 uses this by default
  qputenv("QSG_RENDER_LOOP", "threaded");

  // create a kernel
  epResult r = kernel::Kernel::Create(&pKernel, udParseCommandLine(argc, argv), 8);
  if (r == epR_Failure)
  {
    // TODO: improve error handling/reporting
    udDebugPrintf("Error creating Kernel\n");
    return 1;
  }

  pKernel->RegisterMessageHandler("init", &Init);
  pKernel->RegisterMessageHandler("deinit", &Deinit);

  if (pKernel->RunMainLoop() != epR_Success)
  {
    // TODO: improve error handling/reporting
    udDebugPrintf("Error encountered in Kernel::RunMainLoop()\n");
    return 1;
  }

  udMemoryOutputLeaks();
  udMemoryDebugTrackingDeinit();

  return 0;
}
