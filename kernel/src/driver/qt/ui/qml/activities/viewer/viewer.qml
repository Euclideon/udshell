import QtQuick 2.4
import QtQuick.Controls 1.3
import udKernel 0.1

Item {
  id: viewer
  anchors.fill: parent
  anchors.margins: 10

  property var viewport

  onViewportChanged: viewport.get("uihandle").parent = viewer

  onActiveFocusChanged: {
    if(activeFocus)
      viewport.get("uihandle").forceActiveFocus();
  }
}
