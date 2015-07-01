#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "quick/renderview.h"

#include "udResult.h"
#include "udKernel.h"

udKernel *s_pKernel;

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

  // run Qt app
  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;

  qmlRegisterType<RenderView>("udShell", 0, 1, "RenderView");

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

  return app.exec();
}
