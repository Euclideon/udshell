import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import "qrc:/qml/epcontrols" 0.1

Button {
  style: epToolButtonStyle

  Component {
    id: epToolButtonStyle
    ButtonStyle {
      padding {
        left: 4
        right: 4
        top: 4
        bottom: 4
      }
      background: Rectangle {
        anchors.fill: parent
        color: control.pressed ? "#333" : (control.hovered ? "#888" : "transparent")
      }
      label: Image {
        source: control.iconSource
        fillMode: Image.PreserveAspectFit
      }
    }
  }
}
