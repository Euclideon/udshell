import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

Item {
  id: menuBarWrapper
  implicitWidth: menuBar.__contentItem.width
  implicitHeight: menuBar.__contentItem.height

  property alias menuBar: menuBar
  property var menucomp
  property var commandmanager
  property var window
  property var style
  property var menuObjects: []

  onMenucompChanged: {
    updatemenubar();
    menucomp.subscribe("changed", updatemenubar);
  }

  function updatemenubar() {
    clearAll();

    var menudata = menucomp.get("menudata");

    if(menudata)
    {
      for(var i = 0; i < menudata.length; i++)
        addTopLevelMenu(menudata[i]);
    }
  }

  function clearAll() {
    for(var i = 0; i < menuBar.topMenus.length; i++)
    {
      var menu = menuBar.topMenus[i];
      var menuLength = menu.items.length;
      for(var j = 0; j < menuLength; j++)
        menu.removeItem(menu.items[0]);

      menu.visible = false;
    }
  }

  function addTopLevelMenu(menudata) {
    var menu = menuBar.addMenu(menudata.name);
    var children = menudata.children;
    for(var i = 0; i < children.length; i++)
      menu.addEPMenuItem(children[i]);
  }

  Binding {
    property: "parent"
    target: menuBar ? menuBar.__contentItem : null
    when: menuBar && !menuBar.__isNative
    value: menuBarWrapper
  }
  Binding { target: menuBar; property: "__parentWindow"; value: window }
  Keys.forwardTo: menuBar ? [menuBar.__contentItem] : []

  Binding { target: menuBar; property: "style"; value: style }

  MenuBar {
    id: menuBar
    style: menuBarStyle

    EPMenu { id: menu00; visible: false; }
    EPMenu { id: menu01; visible: false; }
    EPMenu { id: menu02; visible: false; }
    EPMenu { id: menu03; visible: false; }
    EPMenu { id: menu04; visible: false; }
    EPMenu { id: menu05; visible: false; }
    EPMenu { id: menu06; visible: false; }
    EPMenu { id: menu07; visible: false; }
    EPMenu { id: menu08; visible: false; }
    EPMenu { id: menu09; visible: false; }
    EPMenu { id: menu10; visible: false; }
    EPMenu { id: menu11; visible: false; }
    EPMenu { id: menu12; visible: false; }
    EPMenu { id: menu13; visible: false; }
    EPMenu { id: menu14; visible: false; }
    EPMenu { id: menu15; visible: false; }
    EPMenu { id: menu16; visible: false; }
    EPMenu { id: menu17; visible: false; }
    EPMenu { id: menu18; visible: false; }
    EPMenu { id: menu19; visible: false; }

    property variant topMenus: [ menu00, menu01, menu02, menu03, menu04,
                                  menu05, menu06, menu07, menu08, menu09,
                                  menu10, menu11, menu12, menu13, menu14,
                                  menu15, menu16, menu17, menu18, menu19 ]
    property int currMenu: 0

    function addMenu(text)
    {
      if (currMenu == topMenus.length)
        console.log("Index out of range");
      else
      {
        var menu = topMenus[currMenu];
        menu.visible = true;
        menu.title = text;
        menu.commandmanager = Qt.binding(function() { return commandmanager; })
        currMenu++;
        return menu;
      }
    }
  }

  Component {
    id: menuBarStyle
    MenuBarStyle {
      background: Rectangle {
        color: "#444"
      }
      itemDelegate: Rectangle {  // the menus
        implicitWidth: menuBarLabel.contentWidth * 2.0
        implicitHeight: menuBarLabel.contentHeight * 1.4
        color: styleData.selected || styleData.open ? "red" : "transparent"
        Label {
          id: menuBarLabel
          anchors.horizontalCenter: parent.horizontalCenter
          anchors.verticalCenter: parent.verticalCenter
          color: styleData.selected  || styleData.open ? "white" : "white"
          font.wordSpacing: 10
          text: styleData.text
        }
      }
    }
  }
}
