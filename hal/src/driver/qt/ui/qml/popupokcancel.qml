import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1

Rectangle {
  id: popupOKCancel
  visible: false
  enabled: false
  color: "white"
  border.color: "black"
  border.width: 1
  radius: 10

  property alias source: content.source
  property alias sourceComponent: content.sourceComponent

  signal opened()
  signal closed()
  signal ok()
  signal cancel()

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
      cancel();
      close();
    }
    else if(event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
    {
      event.accepted = true;
      ok();
      close();
    }
  }

  Loader {
    id: content
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.margins: parent.border.width
  }

  RowLayout {
    id: okCancelButtons
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.bottomMargin: border.width + 8
    anchors.rightMargin: border.width + 8
    spacing: 7

    Component {
      id: okCancelButtonStyle
      ButtonStyle {
        label: Text {
          font.pixelSize: 11
          text: control.text
          verticalAlignment: Text.AlignVCenter
          horizontalAlignment: Text.AlignHCenter
        }
        background: Rectangle {
          implicitWidth: 50
          border.width: control.activeFocus ? 2 : 1
          border.color: "#888"
          radius: 4
          gradient: Gradient {
            GradientStop { position: 0 ; color: control.pressed ? "#ccc" : "#eee" }
            GradientStop { position: 1 ; color: control.pressed ? "#aaa" : "#ccc" }
          }
        }
        padding { left: 7; right: 7; top: 6; bottom: 6 }
      }
    }

    Button {
      id: okButton
      text: "OK"
      isDefault: true
      onClicked:
      {
        ok()
        close()
      }
      style: okCancelButtonStyle
    }

    Button {
      id: cancelButton
      text: "Cancel"
      onClicked: {
        cancel()
        close()
      }
      style: okCancelButtonStyle
    }
  }

  Component.onCompleted: {
    height = content.item.height + content.item.anchors.topMargin + content.item.anchors.bottomMargin + border.width * 2 + okCancelButtons.implicitHeight + okCancelButtons.anchors.topMargin + okCancelButtons.anchors.bottomMargin;
    width = content.item.width + content.item.anchors.leftMargin + content.item.anchors.rightMargin + border.width * 2;
  }
}

