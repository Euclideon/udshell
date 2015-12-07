#include <QLoggingCategory>
#include <QQuickItem>
#include "driver/qt/util/typeconvert_qt.h"

#include "kernel.h"
#include "components/logger.h"
#include "components/window.h"
#include "components/ui.h"
#include "components/uiconsole.h"
#include "components/project.h"
#include "components/activities/viewer.h"
#include "hal/debugfont.h"

using namespace kernel;

static kernel::Kernel *pKernel = nullptr;
static WindowRef spMainWindow;
static ActivityRef spActiveActivity;

static ProjectRef spProject;

MutableString256 projFilePath;

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

void AddUIActivity(UIComponentRef spTopLevelUI, ActivityRef spActivity)
{
  auto spUI = spActivity->GetUI();

  MutableString<128> title;
  title.append(spActivity->uid);
  title[0] = toUpper(title[0]);

  spTopLevelUI->CallMethod("addactivity", String(spActivity->uid), String(title), spUI);
}

void SetUIConsole(UIComponentRef spTopLevelUI, UIComponentRef spUIConsole)
{
  spTopLevelUI->SetProperty("uiconsole", spUIConsole);
}

void OnActivityChanged(String uid)
{
  if (spActiveActivity)
  {
    spActiveActivity->Deactivate();
    spActiveActivity = nullptr;
  }

  ActivityRef spActivity = pKernel->FindComponent(uid);
  if (!spActivity)
  {
    pKernel->LogError("Unable to activate Activity \"{0}\". Component does not exist", uid);
    return;
  }

  spActiveActivity = spActivity;
  spActivity->Activate();
}

void Deinit(String sender, String message, const Variant &data)
{
  //spProject->SaveProject(); // Uncomment this if you want to dump the state of the Activities into a project file on program close

  spActiveActivity = nullptr;
  spMainWindow = nullptr;
  spProject = nullptr;
}

void Init(String sender, String message, const Variant &data)
{
  if (!projFilePath.empty())
    spProject = pKernel->CreateComponent<Project>({ { "src", String(projFilePath) } });
  else
    spProject = pKernel->CreateComponent<Project>();

  spMainWindow = pKernel->CreateComponent<Window>({ { "file", "qrc:/qml/window.qml" } });
  if (!spMainWindow)
  {
    pKernel->LogError("Error creating MainWindow UI Component\n");
    return;
  }

  auto spTopLevelUI = pKernel->CreateComponent<UIComponent>({ { "file", "qrc:/qml/main.qml" } });
  if (!spTopLevelUI)
  {
    pKernel->LogError("Error creating top Level UI Component\n");
    return;
  }

  auto spConsole = pKernel->CreateComponent<UIConsole>({ { "file", "qrc:/kernel/console.qml" } });
  if (!spConsole)
  {
    pKernel->LogError("Error creating top Level UI Component\n");
    return;
  }
  SetUIConsole(spTopLevelUI, spConsole);

  // Load Activities from project file
  if (spProject)
  {
    auto spActivities = spProject->GetActivities();
    for (auto spActivity : spActivities)
      AddUIActivity(spTopLevelUI, spActivity);
  }

  // Hardcode an example Activity in for testing purposes
  Variant::VarMap cameraParams {
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
    AddUIActivity(spTopLevelUI, spExampleActivity);
    spProject->AddActivity(spExampleActivity);
  }
  else
    pKernel->LogError("Error creating Viewer activity\n");

  spTopLevelUI->Subscribe("activitychanged", Delegate<void(String)>(&OnActivityChanged));

  spMainWindow->SetTopLevelUI(spTopLevelUI);
  // Set ui components
  spTopLevelUI->SetProperty("simplecamera", pKernel->FindComponent("simplecamera0"));
  spTopLevelUI->SetProperty("view", pKernel->FindComponent("view0"));
}

// ---------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  udMemoryDebugTrackingInit();

  // unit test
  epSlice_Test();
  epString_Test();
  epSharedPtr_Test();
  epVariant_Test();

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
