import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1

Rectangle {
  id: popup
  property alias source: content.source
  property alias sourceComponent: content.sourceComponent

  visible: false
  enabled: false
  color: "white"
  border.color: "black"
  border.width: 1
  radius: 10

  signal opened()
  signal closed()

  function close() {
    visible = enabled = false;
    closed();
  }

  function open() {
    opened();
    visible = enabled = true;
    forceActiveFocus();
  }

  Keys.onPressed: {
    if(event.key === Qt.Key_Escape)
    {
      event.accepted = true;
      close();
    }
    else if(event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
    {
      event.accepted = true;
      close();
    }
  }

  Button {
    id: closeButton
    text: "X"
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.topMargin: border.width + 2
    anchors.rightMargin: border.width + 4
    onClicked: close()
    style: ButtonStyle {
      label: Text {
        font.pixelSize: 11
        text: control.text
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
      }
      background: Rectangle {
        border.width: control.activeFocus ? 2 : 1
        border.color: "#888"
        radius: 4
        gradient: Gradient {
          GradientStop { position: 0 ; color: control.pressed ? "#ccc" : "#eee" }
          GradientStop { position: 1 ; color: control.pressed ? "#aaa" : "#ccc" }
        }
      }
      padding { left: 3; right: 3; top: 2; bottom: 2 }
    }
  }

  Loader {
    id: content
    anchors.top: closeButton.bottom
    anchors.left: parent.left
    anchors.margins: parent.border.width
  }

  Component.onCompleted: {
    height = content.item.height + content.item.anchors.topMargin + content.item.anchors.bottomMargin + border.width * 2 + closeButton.implicitHeight + closeButton.anchors.topMargin + closeButton.anchors.bottomMargin;
    width = content.item.width + content.item.anchors.leftMargin + content.item.anchors.rightMargin + border.width * 2;
  }
}

