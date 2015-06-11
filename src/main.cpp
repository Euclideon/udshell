#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "quick/renderview.h"

int main(int argc, char *argv[])
{
  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;

  qmlRegisterType<RenderView>("udShell", 0, 1, "RenderView");

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

  return app.exec();
}
