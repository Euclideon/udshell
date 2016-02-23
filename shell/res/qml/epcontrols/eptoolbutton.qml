import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import epThemes 0.1

Button {
  style: epToolButtonStyle

  Component {
    id: epToolButtonStyle
    ButtonStyle {
      padding {
        left: Theme.toolButtonPadding
        right: Theme.toolButtonPadding
        top: Theme.toolButtonPadding
        bottom: Theme.toolButtonPadding
      }
      background: Rectangle {
        anchors.fill: parent
        color: control.pressed ? Theme.toolButtonBgColorPressed : (control.hovered ? Theme.toolButtonBgColorHovered : Theme.toolButtonBgColor)
      }
      label: Image {
        source: control.iconSource
        fillMode: Image.PreserveAspectFit
      }
    }
  }
}
