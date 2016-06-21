import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2
import Platform 0.1

FocusScope {
  id: %{ProjectName}UI
  anchors.fill: parent
  anchors.topMargin: 4
  anchors.bottomMargin: 4

  property var epTypeDesc: { "super": "ep.uicomponent", "id": "%{Namespace}.mainui" }
  property var viewport

  onViewportChanged: viewport.uihandle.parent = viewPanel
  
  Item {
    id: viewPanel
    anchors.fill: parent
    anchors.rightMargin: 8
  }
}