import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import udKernel 0.1

Rectangle {
  id: topLevel
  anchors.fill: parent
  color: "#444"

  property var uiconsole
  property var activityidlist: []
  property var activityuilist: []

  signal activitychanged(string uid)

  onUiconsoleChanged: {
    var uiconsoleqq = uiconsole.get("uihandle");
    uiconsoleqq.parent = consolePanel;
  }

  onParentChanged: {
    if(activityTabs.count > 0)
    {
      activitychanged(activityidlist[activityTabs.currentIndex]);
      var activityqq = activityuilist[activityTabs.currentIndex].get("uihandle");
      activityqq.visible = false; // Trigger an onVisibleChanged signal
      activityqq.visible = true;
      activityqq.forceActiveFocus();
    }
  }

  function addactivity(uid, title, ui) {
    var tab = activityTabs.addTab(title);
    activityidlist.push(uid);
    activityuilist.push(ui);
    ui.get("uihandle").parent = tab;
  }

  Binding {
    property: "parent"
    target: menuBar ? menuBar.__contentItem : null
    when: menuBar && !menuBar.__isNative
    value: menuBarWrapper
  }
  Binding { target: menuBar; property: "__parentWindow"; value: topLevel.parent }
  Keys.forwardTo: menuBar ? [menuBar.__contentItem] : []

  ColumnLayout {
    id: topLevelColumn
    anchors.fill: parent
    spacing: 0

    Item {
      Layout.fillWidth: true
      Layout.preferredHeight: menuBar.__contentItem.height
      id: menuBarWrapper
      MenuBar {
        id: menuBar
        style: menuBarStyle

        Menu {
          title: "Window"
          MenuItem {
            text: "SubMenu3"
            shortcut: "Ctrl+s"
          }
          MenuItem {
            text: "SubMenu2"
            shortcut: "Ctrl+p"
          }
          MenuItem {
            text: "Preferences"
            shortcut: "Ctrl+,"
          }
        }
      }
    }

    Loader { sourceComponent: separator; Layout.fillWidth: true }

    ToolBar {
      id: toolBar
      Layout.fillWidth: true
      RowLayout {
        height: 30
        width: parent.Layout.preferredWidth
      }
      style: toolBarStyle
    }

    Loader { sourceComponent: separator; Layout.fillWidth: true }

    Item {
      Layout.fillWidth: true
      Layout.fillHeight: true

      TabView {
        id: activityTabs
        objectName: "activityTabs"
        anchors.fill: parent
        anchors.topMargin: 8
        frameVisible: false
        style: tabViewStyle

        Rectangle {
          anchors.fill: parent
          color: "#333"
        }

        onCurrentIndexChanged: {
          activitychanged(activityidlist[currentIndex]);
          activityuilist[activityTabs.currentIndex].get("uihandle").forceActiveFocus();
        }
      }
    }

    ToolBar {
      id: bottomBar
      Layout.fillWidth: true
      RowLayout {
        width: parent.Layout.preferredWidth

        Button {
          id: consoleButton
          text: "Console"
          onClicked: uiconsole.get("uihandle").togglevisible();
          style: bottomBarButtonStyle
        }
      }
      style: toolBarStyle
    }

    Component {
      id: toolBarStyle
      ToolBarStyle {
        padding {
          left: 2
          right: 2
          top: 2
          bottom: 2
        }
        background: Rectangle {
          color: "#444"
        }
      }
    }

    Component {
      id: bottomBarButtonStyle
      ButtonStyle {
        label: Text {
          font.pixelSize: 11
          text: control.text
          color: "white"
          verticalAlignment: Text.AlignVCenter
          horizontalAlignment: Text.AlignHCenter
        }
        background: Rectangle {
          implicitWidth: 50
          border.width: control.activeFocus ? 2 : 1
          border.color: "#333"
          gradient: Gradient {
            GradientStop { position: 0 ; color: control.hovered ? "#333" : "#555" }
            GradientStop { position: 1 ; color: control.hovered ? "#000" : "#333" }
          }
        }
        padding { left: 7; right: 7; top: 4; bottom: 4 }
      }
    }

    Component {
      id: menuBarStyle
      MenuBarStyle {
        background: Rectangle {
          color: "#444"
        }
        itemDelegate: Rectangle {  // the menus
          implicitWidth: menuBarLabel.contentWidth * 1.4
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

    Component {
      id: tabViewStyle
      TabViewStyle {
        tab: Item {
          implicitWidth: bg.implicitWidth
          implicitHeight: Math.max(text.height + 8, 20)
          Rectangle {
            id: bg
            color: styleData.selected ? "#333" : "#555"
            anchors.bottom: parent.bottom
            anchors.bottomMargin: styleData.selected ? 0 : 2
            implicitWidth: Math.max(text.width + 4, 80)
            implicitHeight: styleData.selected ? Math.max(text.height + 8, 20) : Math.max(text.height + 8, 20) - 4
            radius: 2
            Text {
              id: text
              anchors.centerIn: parent
              text: styleData.title
              color: styleData.selected ? "white" : "black"
            }
          }
        }
      }
    }

    Component {
      id: separator
      Rectangle {
        implicitWidth: 100
        implicitHeight: 1
        color: "#555"
      }
    }

    Component {
      id: activityRect
      Rectangle {
        color: "#333"
      }
    }
  }

  Item {
    id: consolePanel
    height: parent.height / 2
    width: parent.width
    anchors.bottom: parent.bottom
    anchors.bottomMargin: bottomBar.height
    anchors.left: parent.left
  }
}
