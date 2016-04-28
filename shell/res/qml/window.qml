import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.3

Window {
  id: desktopwindow
  title: "Euclideon Platform"
  width: 960
  height: 540

  property var epTypeDesc: {
    "super": "ep.window",
    "id": "ui.appwindow",
    "displayname": "UI.AppWindow",
    "description": "QML Based Application Window"
  }
  property alias title: desktopwindow.title
}
