#include <QQmlApplicationEngine>
#include <QDebug>
#include <QLoggingCategory>

#include "udKernel.h"

#include "quick/window.h"

udKernel *s_pKernel;


// ---------------------------------------------------------------------------------------
void DbgMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  // TODO: replace this with something more robust - maybe a full logging system and status console
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


// ---------------------------------------------------------------------------------------
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

  // install our qt message handler
  qInstallMessageHandler(DbgMessageHandler);
  //QLoggingCategory::setFilterRules("qt.*=true");

  // create the main window
  Window w;
  w.setResizeMode(QQuickView::SizeRootObjectToView);
  w.setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  w.resize(800, 600);
  w.show();
  w.raise();

  return s_pKernel->RunMainLoop();
}
