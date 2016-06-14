import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import Platform.Themes 0.1

Button {
  id: button
  style: epToolButtonStyle

  EPToolTip {
    target: button
    text: button.tooltip
  }

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
