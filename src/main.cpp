#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QLoggingCategory>

#include "quick/renderview.h"

#include "udResult.h"
#include "udKernel.h"

udKernel *s_pKernel;

// TODO: Replace this with something more robust - maybe a full logging system and status console
void DbgMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  switch (type) {
  case QtDebugMsg:
    udDebugPrintf("Dbg: %s (%s:%d, %s)\n", msg.toLatin1().data(), context.file, context.line, context.function);
    break;
  case QtWarningMsg:
    udDebugPrintf("Wrn: %s (%s:%d, %s)\n", msg.toLatin1().data(), context.file, context.line, context.function);
    break;
  case QtCriticalMsg:
    udDebugPrintf("Crt: %s (%s:%d, %s)\n", msg.toLatin1().data(), context.file, context.line, context.function);
    break;
  case QtFatalMsg:
    udDebugPrintf("Ftl: %s (%s:%d, %s)\n", msg.toLatin1().data(), context.file, context.line, context.function);
    abort();
  }
}


int main(int argc, char *argv[])
{
  // unit test
  udResult udSlice_Test();
  udSlice_Test();
  udResult udString_Test();
  udString_Test();

  // create a kernel
  udResult r = udKernel::Create(&s_pKernel, udParseCommandLine(argc, argv), 8);
  if (r == udR_Failure_)
    return 1;

  qInstallMessageHandler(DbgMessageHandler);
  //QLoggingCategory::setFilterRules("qt.*=true");

  // create the Qt app
  QGuiApplication app(argc, argv);

  // make sure we cleanup the kernel when we're about to quit
  QObject::connect(&app, &QCoreApplication::aboutToQuit, []{ s_pKernel->Destroy(); });

  // create the qml engine and load our script
  QQmlApplicationEngine engine;
  qmlRegisterType<RenderView>("udShell", 0, 1, "RenderView");
  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

  // run the Qt event loop
  return app.exec();
}
