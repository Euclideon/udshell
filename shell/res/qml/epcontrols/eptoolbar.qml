import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import epKernel 0.1

Rectangle {
  property bool vertical
  property var toolbarcomp
  property var commandManager
  property var toolBarObjects: []
  property alias toolBarLayout: toolBarLoader.item

  Component.onCompleted: {
    commandManager = EPKernel.getCommandManager();
  }

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
      var component = Qt.createComponent("epmenubutton.qml");
      var item = component.createObject(null);
      toolBarObjects.push(item);

      item.buttonHeight = vertical ? toolBar.width : toolBar.height;
      item.buttonWidth = vertical ? toolBar.width: toolBar.height;
      item.text = tooldata.name;
      item.iconSource = tooldata.image;
      item.command = tooldata.command;
      item.enabled = tooldata.enabled;
      item.tooltip = tooldata.description;
      item.split = tooldata.split;
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
    id: toolBarStyle
    ToolBarStyle {
      background: Rectangle {
        color: "#444"
      }
    }
  }
}
