import QtQuick 2.4
import QtQuick.Controls 1.3
import udKernel 0.1

FocusScope {
  id: viewport
  anchors.fill: parent

  UDRenderView {
    id: renderview
    anchors.fill: parent
  }
}
