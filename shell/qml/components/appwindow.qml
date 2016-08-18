import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.5

Window
{
  id: desktopWindow
  title: "Euclideon Platform"
  width: 960
  height: 540

  // Properties // ------------------------------------------------------------
  property var epTypeDesc: {
    "id": "ui.AppWindow",
    "super": "ep.Window",
    "description": "QML Based Application Window"
  }
  property alias title: desktopWindow.title
}
