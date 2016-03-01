import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import epKernel 0.1

RowLayout {
  id: epMenuButton
  property alias text: button.text
  property alias iconSource: button.iconSource
  property alias command: button.command
  property alias tooltip: button.tooltip
  property alias buttonHeight: button.implicitHeight
  property alias buttonWidth: button.implicitWidth
  property bool split: true
  property var commandManager
  spacing: 1

  Component.onCompleted: {
    commandManager = EPKernel.getCommandManager();
  }

  function addItem(data) {
    menu.addEPMenuItem(data);
    if(menu.items.length == 1)
      menu.__selectedIndex = 0;
  }

  Button {
    id: button
    property string command
    property alias dropdown: dropdown
    property alias menu: menu

    implicitHeight: 40
    implicitWidth: 40

    onClicked: {
      if(split) {
        if(command)
          commandManager.call("runcommand", command, null);
      }
      else if(dropdown.visible) {
        menu.toggleVisible();
      }
    }

    style: menuButtonStyle

    EPToolTip {
      target: button
      text: button.tooltip
    }
  }

  Button {
    id: dropdown
    visible: (menu.items.length > 0 ? true : false)

    property alias button: button
    property alias menu: menu
    style: menuButtonDropDownStyle

    onClicked: menu.toggleVisible();
  }

  EPMenu
  {
    id: menu

    __visualItem: epMenuButton

    on__SelectedIndexChanged: {
      var item = items[__selectedIndex];
      if(item.iconSource)
        button.iconSource = item.iconSource;
      if(item.command)
        button.command = item.command;
    }

    function toggleVisible() {
      if(__popupVisible)
        __dismissMenu()
      else
        __popup(Qt.rect(0, buttonHeight, 0, 0), 0);
    }
  }

  Component {
    id: menuButtonDropDownStyle
    ButtonStyle {
      label: Item {}
      background: Rectangle {
        id: glyph
        implicitWidth: arrow.implicitWidth + 2
        implicitHeight: control.button.implicitHeight
        color: !control.enabled ? "transparent" : (control.pressed ? "#333" : (control.hovered || control.button.hovered || control.menu.__popupVisible ? "#555" : "transparent"))
        Rectangle {
          width: arrow.implicitWidth
          height: arrow.implicitHeight
          color: "transparent"
          anchors.verticalCenter: parent.verticalCenter
          anchors.horizontalCenter: parent.horizontalCenter

          Text {
            id: arrow
            font.pixelSize: 9
            text: control.menu.__popupVisible ? "\u25b2" : "\u25bc"
            color: "white"
          }
        }
      }
    }
  }

  Component {
    id: menuButtonStyle
    ButtonStyle {
      padding {
        left: 2
        right: 2
        top: 2
        bottom: 2
      }
      background: Rectangle {
        anchors.fill: parent
        color: control.pressed ? "#333" : ((control.hovered || control.dropdown.hovered || control.menu.__popupVisible) ? "#555" : "transparent")
      }
      label: Image {
        source: control.iconSource
        fillMode: Image.PreserveAspectFit
      }
    }
  }
}

