import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import epKernel 0.1
import epThemes 0.1

Menu {
  id: epMenu
  style: menuStyle

  property var menuObjects: []
  property var commandManager

  Component.onCompleted: {
    commandManager = EPKernel.getCommandManager();
  }

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
          commandManager.call("runcommand", command, null);
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

      frame: Rectangle {
        color: Theme.menuItemBgColor
      }

      itemDelegate {
        background: Rectangle {
          color: styleData.enabled && (styleData.selected || styleData.open) ? Theme.menuItemBgColorSelected : Theme.menuItemBgColor
          radius: styleData.selected ? Theme.menuItemBgRadiusSelected : Theme.menuItemBgRadius
        }

        label: Label {
          color: !styleData.enabled ? Theme.menuItemDisabledColor : (styleData.selected ? Theme.menuItemTextColorSelected : Theme.menuItemTextColorSelected)
          text: styleData.text
        }

        submenuIndicator: Text {
          text: "\u25ba"
          font: mStyle.font
          color: !styleData.enabled ? Theme.menuItemDisabledColor : (styleData.selected || styleData.open ? Theme.menuItemSubmenuIndicatorColorSelected : Theme.menuItemSubmenuIndicatorColor)
          styleColor: Qt.lighter(color, 4)
        }

        shortcut: Label {
          color: !styleData.enabled ? Theme.menuItemDisabledColor : (styleData.selected ? Theme.menuItemShortcutTextColorSelected : Theme.menuItemShortcutTextColor)
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
              border.color: !styleData.enabled ? Theme.menuItemDisabledColor : Theme.menuItemCheckboxBorderColor
              border.width: 2
              Rectangle {
                visible: control.checked
                color: !styleData.enabled ? Theme.menuItemDisabledColor : Theme.menuItemCheckboxCheckColor
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
          color: Theme.menuItemSeparatorColor
        }
      }
    }
  }
}
