import QtQuick 2.4
import QtQuick.Controls 1.3
import udKernel 0.1

Item {
  id: main
  width: 100
  height: 100

  RenderView {
    id: viewport
    anchors.fill: parent
    anchors.margins: 10
  }
}
