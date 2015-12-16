import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1

Rectangle {
  property bool vertical
  property var toolbarcomp
  property var commandmanager
  property var toolBarObjects: []
  property alias toolBarLayout: toolBarLoader.item

  onToolbarcompChanged: {
    updatetoolbar();
    toolbarcomp.subscribe("changed", updatetoolbar);
  }

  function updatetoolbar() {
    clearAll();

    var toolBarData = toolbarcomp.get("menudata");

    if(toolBarData)
    {
      for(var i = 0; i < toolBarData.length; i++)
        addToolBarItem(toolBarData[i]);
    }
  }

  function clearAll() {
    for(var i = 0; i < toolBarObjects.length; i++)
    {
      var item = toolBarObjects[i];
      item.parent = null;
      //item.destroy(); // Should destroy the buttons but this causes warning spam due to qt bug
    }
    toolBarObjects = [];
  }

  function addToolBarItem(tooldata) {
    if(tooldata.type == "separator")
    {
      var item = separator.createObject(null);
      toolBarObjects.push(item);

      item.parent = toolBarLayout;
    }
    else
    {
      var item = epSplitButton.createObject(null);
      toolBarObjects.push(item);

      item.text = tooldata.name;
      item.iconSource = tooldata.image;
      item.command = tooldata.command;
      item.enabled = tooldata.enabled;
      item.tooltip = tooldata.description;
      item.commandmanager = Qt.binding(function() { return commandmanager; });
      item.parent = toolBarLayout;

      var children = tooldata.children;
      for(var i = 0; i < children.length; i++)
        item.addItem(children[i]);
    }
  }

  id: toolBar
  implicitHeight: 1
  implicitWidth: 1

  color: "transparent"

  Loader {
    id: toolBarLoader
    sourceComponent: vertical ? verticalLayout : horizontalLayout
  }

  Component
  {
    id: horizontalLayout
    RowLayout {
      anchors.verticalCenter: parent.verticalCenter
      spacing: 0
    }
  }

  Component
  {
    id: verticalLayout
    ColumnLayout {
      anchors.verticalCenter: parent.verticalCenter
      spacing: 0
    }
  }

  Component {
    id: epSplitButton

    RowLayout {
      id: splitButton
      property alias text: button.text
      property alias iconSource: button.iconSource
      property alias command: button.command
      property alias tooltip: button.tooltip
      property var commandmanager: null
      spacing: 1

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

        implicitHeight: vertical ? toolBar.width : toolBar.height
        implicitWidth: vertical ? toolBar.width: toolBar.height

        onClicked: {
          if(command)
            commandmanager.call("runcommand", command, null);
        }

        style: splitButtonStyle
      }

      Button {
        id: dropdown
        visible: (menu.items.length > 0 ? true : false)

        property alias button: button
        property alias menu: menu
        style: splitButtonDropDownStyle

        onClicked: {
          menu.toggleVisible();
        }
      }

      EPMenu
      {
        id: menu

        __visualItem: splitButton
        commandmanager: splitButton.commandmanager

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
            __popup(Qt.rect(0, splitButton.height, 0, 0), 0);
        }
      }
    }
  }

  Component {
    id: separator
    Item {
      implicitWidth: vertical ? toolBar.implicitWidth : 14
      implicitHeight: vertical ? 14 : toolBar.implicitHeight
      Rectangle {
        anchors.centerIn: parent
        implicitWidth: vertical ? toolBar.implicitWidth - 8 : 1
        implicitHeight: vertical ? 1 : toolBar.implicitHeight - 8
        color: "#555"
      }
    }
  }

  Component {
    id: splitButtonDropDownStyle
    ButtonStyle {
      label: Item {}
      background: Rectangle {
        id: glyph
        implicitWidth: arrow.implicitWidth + 2
        implicitHeight: control.button.implicitHeight
        color: !control.enabled ? "transparent" : (control.hovered || control.button.hovered || control.menu.__popupVisible ? "#555" : "transparent")
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
    id: splitButtonStyle
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

  Component {
    id: toolBarStyle
    ToolBarStyle {
      background: Rectangle {
        color: "#444"
      }
    }
  }
}
