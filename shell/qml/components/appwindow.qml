import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.5

Window {
  id: desktopwindow
  title: "Euclideon Platform"
  width: 960
  height: 540

  property var epTypeDesc: {
    "id": "ui.AppWindow",
    "super": "ep.Window",
    "displayname": "UI.AppWindow",
    "description": "QML Based Application Window"
  }
  property alias title: desktopwindow.title
}
