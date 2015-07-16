import QtQuick 2.4
import QtQuick.Controls 1.3
import udShell 0.1

Item {
  id: desktopwindow
  width: 640
  height: 480

  RenderView {
    id: viewport
    anchors.fill: parent
    anchors.margins: 10
  }
}
