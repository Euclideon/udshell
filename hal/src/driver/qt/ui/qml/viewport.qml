import QtQuick 2.4
import QtQuick.Controls 1.3
import epKernel 0.1

FocusScope {
  id: viewport
  anchors.fill: parent

  property var epTypeDesc: {
    "super": "ep.viewport",
    "id": "ui.viewport",
    "displayname": "UI.Viewport",
    "description": "QML Based Viewport"
  }

  EPRenderView {
    id: renderview
    anchors.fill: parent

    onActiveFocusChanged: {
      if(activeFocus) {
        EPKernel.focus.push(this);
      }
    }
  }
}
