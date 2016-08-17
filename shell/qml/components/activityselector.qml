import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2
import Platform.Controls 0.1

Item
{
  // Properties // ------------------------------------------------------------
  property var epTypeDesc: { "id": "ui.ActivitySelector", "super": "ep.UIComponent" }
  property var activitiesInfo: []

  // Methods // ---------------------------------------------------------------
  function show(cb)
  {
    internal.callback = cb;

    infoModel.clear();
    for(var i = 0; i < activitiesInfo.length; i++) {
      var text = "<font size=\"6\" color=\"lightskyblue\">" + activitiesInfo[i].name + "</font><br><br>" + activitiesInfo[i].description + "<br>";

      activitiesInfo[i]["formattedText"] = text;
      infoModel.append(activitiesInfo[i]);
    }
    //infoModel.sort();

    listView.currentIndex = -1;

    win.show();
  }

  // Internal // --------------------------------------------------------------
  QtObject
  {
    id: internal
    property var callback
  }

  // Item Tree // -------------------------------------------------------------
  EPListModel {
    id: infoModel
  }

  Window {
    id: win

    title: "Choose an Activity Type"
    width: 400
    height: 400
    flags: Qt.Window | Qt.WindowSystemMenuHint | Qt.WindowTitleHint | Qt.WindowCloseButtonHint
    modality: Qt.WindowModal

    Rectangle {
      anchors.fill: parent
      color: "#666"
      ColumnLayout {
        anchors.fill: parent
        Item {
          Layout.fillWidth: true
          Layout.fillHeight: true
          Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            color: "#333"
            EPListView {
              id: listView
              alternatingRowColors: true
              anchors.fill: parent
              model: infoModel
              modelDisplayItem: "formattedText"
            }
          }
        }

        Item {
          Layout.fillWidth: true
          Layout.preferredHeight: buttonLayout.implicitHeight + buttonLayout.anchors.topMargin + buttonLayout.anchors.bottomMargin
          RowLayout {
            id: buttonLayout
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.rightMargin: 10
            Button {
              id: okButton
              text: "OK"

              onClicked: {
                win.close();
                internal.callback(listView.selectedItemData.identifier);
              }
            }

            Button {
              id: cancelButton
              text: "Cancel"

              onClicked: {
                win.close();
              }
            }
          }
        }
      }
    }
  }
}
