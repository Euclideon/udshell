import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

Menu {
  id: epMenu
  style: menuStyle

  property var menuObjects: []
  property var commandmanager

  Component {
    id: epExclusiveGroup
    ExclusiveGroup {}
  }

  Component {
    id: epMenuItem
    MenuItem {
      property string command
      onTriggered: {
        if(command)
        {
          commandmanager.call("runcommand", command, null);
        }
      }
    }
  }

  function addEPMenuItem(menudata) {
    var children = menudata.children;

    if(menudata.type == "separator")
      addSeparator();
    else if(children.length > 0)
    {
      var component = Qt.createComponent("epmenu.qml");
      var menu = component.createObject(null);
      menuObjects.push(menu);
      menu.title = menudata.name;
      menu.commandmanager = Qt.binding(function() { return commandmanager; })
      insertItem(items.length, menu);

      for(var i = 0; i < children.length; i++)
      {
        menu.addEPMenuItem(children[i]);
      }
    }
    else
    {
      var menuItem = epMenuItem.createObject(null);
      menuObjects.push(menuItem);
      menuItem.text = menudata.name;
      menuItem.iconSource = menudata.image;
      menuItem.command = menudata.command;
      menuItem.shortcut = menudata.shortcut;
      menuItem.checkable = menudata.checkable;
      menuItem.enabled = menudata.enabled;
      if(menudata.checkable)
      {
        menuItem.checked = menudata.checked;
        if(menudata.exclusivegroup)
        {
          if(items.length)
          {
            var lastItem = items[items.length - 1];
            if(!lastItem.exclusiveGroup)
              lastItem.exclusiveGroup = epExclusiveGroup.createObject(menuBarWrapper);

            menuItem.exclusiveGroup = lastItem.exclusiveGroup;
          }
        }
      }
      insertItem(items.length, menuItem)
    }
  }

  Component {
    id: menuStyle
    MenuStyle {               // the menus items
      id: mStyle
      property color menuBackgroundColor: "#1B1B1B"
      property color menuBorderColor: "#282828"

      frame: Rectangle {
        color: menuBackgroundColor
      }

      itemDelegate {
        background: Rectangle {
          color:  styleData.selected || styleData.open ? "#3C3C3C" : menuBackgroundColor
          radius: styleData.selected ? 3 : 0
        }

        label: Label {
          color: styleData.selected ? "white" : "white"
          text: styleData.text
        }

        submenuIndicator: Text {
          text: "\u25ba"
          font: mStyle.font
          color: styleData.selected  || styleData.open ? "white" : "white"
          styleColor: Qt.lighter(color, 4)
        }

        shortcut: Label {
          color: styleData.selected ? "white" : "white"
          text: styleData.shortcut
        }

        checkmarkIndicator: CheckBox {          // not strinctly a Checkbox. A Rectangle is fine too
          checked: styleData.checked
          style: CheckBoxStyle {
            indicator: Rectangle {
              implicitWidth: mStyle.font.pixelSize
              implicitHeight: implicitWidth
              radius: 2
              color: menuBackgroundColor
              border.color: "#555"
              border.width: 2
              Rectangle {
                visible: control.checked
                color: "white"
                anchors.margins: 3
                radius: 2
                anchors.fill: parent
              }
            }
            spacing: 10
          }
        }
      }

      // scrollIndicator: // <--- could be an image

      separator: Item {
        implicitHeight: 5
        width: parent.width
        Rectangle {
          anchors.verticalCenter: parent.verticalCenter
          width: parent.width
          implicitHeight: 1
          color: "#555"
        }
      }
    }
  }
}
