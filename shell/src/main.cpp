#include <QLoggingCategory>
#include <QQuickItem>
#include "driver/qt/util/typeconvert_qt.h"

#include "kernel.h"
#include "components/logger.h"
#include "components/window.h"
#include "components/ui.h"
#include "components/uiconsole.h"
#include "components/activities/viewer.h"
#include "hal/debugfont.h"

static Kernel *pKernel = nullptr;
static WindowRef spMainWindow;
static ActivityRef spActiveActivity;

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
  spActiveActivity = nullptr;
  spMainWindow = nullptr;
}

void Init(String sender, String message, const Variant &data)
{
  // TODO: load a project file...
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

  auto spViewerActivity = pKernel->CreateComponent<Viewer>();
  AddUIActivity(spTopLevelUI, spViewerActivity);

  spTopLevelUI->Subscribe("activitychanged", Delegate<void(String)>(&OnActivityChanged));
  spMainWindow->SetTopLevelUI(spTopLevelUI);
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

  // install our qt message handler
  qInstallMessageHandler(DbgMessageHandler);
  //QLoggingCategory::setFilterRules("qt.*=true");

  // TEMP: this is just for testing - force qt to use the threaded renderer on windows
  // 5.5 uses this by default
  qputenv("QSG_RENDER_LOOP", "threaded");

  // create a kernel
  epResult r = Kernel::Create(&pKernel, udParseCommandLine(argc, argv), 8);
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
