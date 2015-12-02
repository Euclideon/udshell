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
  property var simplecamera
  property var view
  property var activityidlist: []
  property var activityuilist: []
  property var tablist: [] // Store a strong reference to the tabs to stop TabView from garbage collecting them

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

  function cameraupdated(pos, ypr)
  {
    cameraPosX.text = pos[0].toFixed(2);
    cameraPosY.text = pos[1].toFixed(2);
    cameraPosZ.text = pos[2].toFixed(2);

    cameraYaw.text = ypr[0].toFixed(2);
    cameraPitch.text = ypr[1].toFixed(2);
    cameraRoll.text = ypr[2].toFixed(2);
  }

  function viewmouseupdated(pos)
  {
    mousePosX.text = pos[0];
    mousePosY.text = pos[1];
  }

  function viewpickingenabledchanged(enabled)
  {
    if (enabled)
      pickText.text = "Picking Enabled";
    else
      pickText.text = "Picking Disabled";
  }

  function viewpickfound(pos)
  {
    pickPosX.text = pos[0].toFixed(2);
    pickPosY.text = pos[1].toFixed(2);
    pickPosZ.text = pos[2].toFixed(2);
  }

  onSimplecameraChanged: {
    // We are using the fact that this "on changed event" fires as during the init phase
    simplecamera.subscribe("changed", cameraupdated);
  }

  onViewChanged: {
    // We are using the fact that this "on changed event" fires as during the init phase
    view.subscribe("mousepositionchanged", viewmouseupdated);
    view.subscribe("enabledpickingchanged", viewpickingenabledchanged);
    view.subscribe("pickfound", viewpickfound);
  }

  function addactivity(uid, title, ui) {
    var tab = activityTabs.addTab(title);
    tablist.push(tab);
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
        Text {
          id: cameraPosText
          color: "yellow"
          text: "Camera Pos"
        }
        Text {
          id: cameraPosX
          color: "white"
        }
        Text {
          id: cameraPosY
          color: "white"
        }
        Text {
          id: cameraPosZ
          color: "white"
        }

        Text {
          id: cameraOrientationText
          color: "yellow"
          text: "Camera YPR"
        }
        Text {
          id: cameraYaw
          color: "white"
        }
        Text {
          id: cameraPitch
          color: "white"
        }
        Text {
          id: cameraRoll
          color: "white"
          text: "0.00"
        }

        Text {
          id: mousePosText
          color: "yellow"
          text: "Mouse Pos"
        }
        Text {
          id: mousePosX
          color: "white"
        }
        Text {
          id: mousePosY
          color: "white"
        }

        Text {
          id: pickText
          color: "yellow"
          text: "Picking Disabled"
        }
        Text {
          id: pickPosX
          color: "white"
        }
        Text {
          id: pickPosY
          color: "white"
        }
        Text {
          id: pickPosZ
          color: "white"
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
