import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import epControls 0.1

Item {
  property var activitiesinfo: []
  property var callback

  function show(cb) {
    callback = cb;

    infoModel.clear();
    for(var i = 0; i < activitiesinfo.length; i++) {
      var text = "<font size=\"6\" color=\"lightskyblue\">" + activitiesinfo[i].displayName + "</font><br><br>" + activitiesinfo[i].description + "<br>";

      activitiesinfo[i]["formattedText"] = text;
      infoModel.append(activitiesinfo[i]);
    }
    infoModel.sort();

    listView.currentIndex = -1;

    win.show();
  }

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
                callback(listView.selectedItemData.id);
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
