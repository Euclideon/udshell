import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import QtQuick.Dialogs 1.2
import Platform 0.1
import Platform.Controls 0.1
import Platform.Themes 0.1

Rectangle {
  id: topLevel
  anchors.fill: parent
  color: Theme.windowBgColor

  property var epTypeDesc: { "id": "ui.Main", "super": "ep.UIComponent" }
  property var uiconsole
  property var messageboxcomp
  property var messagebox
  property var menucomp
  //property var toolbarcomp
  property var activityselector

  signal activitychanged(string uid)

  signal newprojectsignal(string path)
  signal openprojectsignal(string path)
  signal saveprojectsignal()
  signal saveprojectassignal(string path)
  signal newactivitysignal(string id)

  function addactivity(activity)
  {
    if(activityTabs.count == 0)
      activityTabs.visible = true;

    var title = activity.uid;
    title = title.charAt(0).toUpperCase() + title.slice(1);

    var ui = activity.ui;

    var tab = activityTabs.addTab(title);
    internal.tabList.push(tab);
    internal.activityList.push(activity);
    internal.activityUIList.push(ui);
    internal.tabList.push(tab);
    ui.uiHandle.parent = tab;

    activityTabs.currentIndex = activityTabs.count - 1;
  }

  function removeactivity(uid)
  {
    for(var i = 0; i < internal.activityList.length; i++)
    {
      if(internal.activityList[i].uid == uid)
      {
        internal.activityList.splice(i, 1);
        internal.activityUIList.splice(i, 1);
        activityTabs.removeTab(i);
        internal.tabList.splice(i, 1);
        break;
      }
    }

    if(activityTabs.count == 0)
    {
      activityTabs.visible = false;
      activityTabs.currentIndex = -1;
    }
  }

  onMessageboxcompChanged: {
    messagebox = messageboxcomp.uiHandle;
    messagebox.parent = topLevel;
  }

  onUiconsoleChanged: {
    uiconsole.uiHandle.parent = consolePanel;
  }

  onMenucompChanged: {
    menuBar.menucomp = menucomp;
  }

  //onToolbarcompChanged: {
    //toolBar.toolbarcomp = toolbarcomp;
  //}

  onActivityselectorChanged: {
    activityselector.uiHandle.parent = this;
  }

  Component.onCompleted: {
    var commandManager = EPKernel.getCommandManager();
    commandManager.registerCommand("newproject", internal.newProject, "", "", "Ctrl+N");
    commandManager.registerCommand("openproject", internal.openProject, "", "", "Ctrl+O");

    commandManager.registerCommand("saveproject", internal.saveProject, "", "", "Ctrl+S");
    commandManager.registerCommand("saveprojectas", internal.saveProjectAs, "", "", "F12");
    commandManager.registerCommand("newactivity", internal.newActivity, "", "", "Ctrl+A");

    commandManager.registerCommand("showhideconsolepanel", internal.showHideConsolePanel, "", "", "`");

    // Disable these shortcuts, they will get enabled when a project is created or opened
    commandManager.disableShortcut("saveproject");
    commandManager.disableShortcut("saveprojectas");
    commandManager.disableShortcut("newactivity");
  }

  QtObject
  {
    id: internal
    property var simpleCamera: null
    property var view: null
    property var activityList: []
    property var activityUIList: []
    property var tabList: [] // Store a strong reference to the tabs to stop TabView from garbage collecting them
    property var fileDialog

    function showHideConsolePanel()
    {
      if (uiconsole)
         uiconsole.togglevisible();
    }

    function cameraUpdated(pos, ypr)
    {
      cameraPosX.text = pos[0].toFixed(2);
      cameraPosY.text = pos[1].toFixed(2);
      cameraPosZ.text = pos[2].toFixed(2);

      cameraYaw.text = ypr[0].toFixed(2);
      cameraPitch.text = ypr[1].toFixed(2);
      cameraRoll.text = ypr[2].toFixed(2);
    }

    function viewMouseUpdated(pos)
    {
      mousePosX.text = pos[0];
      mousePosY.text = pos[1];
    }

    function viewPickingEnabledChanged(enabled)
    {
      if (enabled)
        pickText.text = "Picking Enabled";
      else
        pickText.text = "Picking Disabled";
    }

    function viewPickFound(pos)
    {
      pickPosX.text = pos[0].toFixed(2);
      pickPosY.text = pos[1].toFixed(2);
      pickPosZ.text = pos[2].toFixed(2);
    }

    function initViewerUI(activity)
    {
      if (internal.simpleCamera)
        internal.simpleCamera.repositioned.disconnect(cameraUpdated);

      if(internal.view)
      {
        internal.view.mousePositionChanged.disconnect(viewMouseUpdated);
        internal.view.enabledPickingChanged.disconnect(viewPickingEnabledChanged);
        internal.view.pickFound.disconnect(viewPickFound);
      }

      if(!activity)
      {
        internal.simpleCamera = null;
        internal.view = null;
        return;
      }

      internal.simpleCamera = activity.simpleCamera;
      if (internal.simpleCamera)
        internal.simpleCamera.repositioned.connect(cameraUpdated);

      internal.view = activity.view;
      if (internal.view)
      {
        internal.view.mousePositionChanged.connect(viewMouseUpdated);
        internal.view.enabledPickingChanged.connect(viewPickingEnabledChanged);
        internal.view.pickFound.connect(viewPickFound);
      }
    }

    function newActivity()
    {
      activityselector.uiHandle.show(newActivitySelected);
    }

    function newActivitySelected(id)
    {
      newactivitysignal(id);
    }

    function newProject()
    {
      internal.fileDialog = fileDialogComp.createObject(topLevel);
      internal.fileDialog.title = "Select project location and enter a name for the new project";
      internal.fileDialog.onAccepted.connect(newProjectAccepted);
      internal.fileDialog.selectMultiple = false;
      internal.fileDialog.selectExisting = false;
      internal.fileDialog.nameFilters = ["*.epproj"];
      internal.fileDialog.visible = true;
    }

    function newProjectAccepted()
    {
      internal.fileDialog.destroy();
      newprojectsignal(internal.fileDialog.fileUrl);
    }

    function openProject()
    {
      internal.fileDialog = fileDialogComp.createObject(topLevel);
      internal.fileDialog.title = "Select a Euclideon Platform Project file to open";
      internal.fileDialog.onAccepted.connect(openProjectAccepted);
      internal.fileDialog.selectMultiple = false;
      internal.fileDialog.selectExisting = true;
      internal.fileDialog.nameFilters = ["*.epproj"];
      internal.fileDialog.visible = true;
    }

    function openProjectAccepted()
    {
      internal.fileDialog.destroy();
      openprojectsignal(internal.fileDialog.fileUrl);

    }

    function saveProject()
    {
      saveprojectsignal();
    }

    function saveProjectAs()
    {
      internal.fileDialog = fileDialogComp.createObject(topLevel);
      internal.fileDialog.title = "Select project location and enter a name for the project";
      internal.fileDialog.onAccepted.connect(saveProjectAsAccepted);
      internal.fileDialog.selectMultiple = false;
      internal.fileDialog.selectExisting = false;
      internal.fileDialog.nameFilters = ["*.epproj"];
      internal.fileDialog.visible = true;
    }

    function saveProjectAsAccepted()
    {
      internal.fileDialog.destroy();
      saveprojectassignal(internal.fileDialog.fileUrl);
    }
  }

  ColumnLayout {
    id: topLevelColumn
    anchors.fill: parent
    spacing: 0

    EPMenuBar {
      id: menuBar
      Layout.fillWidth: true
      window: topLevel.parent
    }

    Loader { sourceComponent: separator; Layout.fillWidth: true }

    //EPToolBar {
    //  id: toolBar
    //  Layout.fillWidth: true
    //  color: Theme.windowToolBarBgColor
    //  Layout.preferredHeight: Theme.windowToolBarSize
    //}
    //
    //Loader { sourceComponent: separator; Layout.fillWidth: true }

    Item {
      Layout.fillWidth: true
      Layout.fillHeight: true
      TabView {
        id: activityTabs
        objectName: "activityTabs"
        currentIndex: -1
        visible: false
        anchors.fill: parent
        anchors.topMargin: 4
        anchors.leftMargin: 4
        frameVisible: false
        style: tabViewStyle

        property string lastId: ""

        Rectangle {
          anchors.fill: parent
          color: Theme.tabBgColor
        }

        onCurrentIndexChanged: {
          if(count > 0 && internal.activityList[currentIndex].uid != lastId)
          {
            var activityId = internal.activityList[currentIndex].uid;
            lastId = activityId
            activitychanged(activityId);
            var activity = internal.activityList[currentIndex];
            var activityqq = internal.activityUIList[currentIndex].uiHandle;
            activityqq.visible = false; // Trigger an onVisibleChanged signal
            activityqq.visible = true;
            activityqq.forceActiveFocus();

            internal.initViewerUI(activity);
          }
          else
          {
            lastId = "";
            activitychanged(null);

            internal.initViewerUI(null);
          }
        }
      }
    }

    Loader { sourceComponent: separator; Layout.fillWidth: true }

    ToolBar {
      id: bottomBar
      Layout.fillWidth: true
      RowLayout {
        width: parent.Layout.preferredWidth
        Button {
          id: consoleButton
          text: "Console"
          onClicked: uiconsole.togglevisible();
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
          text: "Picking Enabled"
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
          color: Theme.bottomBarBgColor
        }
      }
    }

    Component {
      id: bottomBarButtonStyle
      ButtonStyle {
        label: Text {
          font.pixelSize: Theme.bottomBarButtonFontSize
          text: control.text
          color: Theme.bottomBarButtonTextColor
          verticalAlignment: Text.AlignVCenter
          horizontalAlignment: Text.AlignHCenter
        }
        background: Rectangle {
          implicitWidth: 50
          border.width: control.activeFocus ? 2 : 1
          border.color: Theme.bottomBarButtonBorderColor
          gradient: Gradient {
            GradientStop { position: 0 ; color: control.hovered ? Theme.bottomBarButtonBgColorHoveredStart : Theme.bottomBarButtonBgColorStart }
            GradientStop { position: 1 ; color: control.hovered ? Theme.bottomBarButtonBgColorHoveredEnd : Theme.bottomBarButtonBgColorEnd }
          }
        }
        padding { left: 7; right: 7; top: 4; bottom: 4 }
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
            color: styleData.selected ? Theme.tabBarBgColorSelected : Theme.tabBarBgColor
            anchors.bottom: parent.bottom
            anchors.bottomMargin: styleData.selected ? 0 : 2
            implicitWidth: text.width + closeButton.width + 16
            implicitHeight: styleData.selected ? Math.max(text.height + 8, 20) : Math.max(text.height + 8, 20) - 4
            radius: Theme.tabBarRadius
            Text {
              id: text
              anchors.verticalCenter: parent.verticalCenter
              anchors.left: parent.left
              anchors.leftMargin: 8
              text: styleData.title
              color: styleData.selected ? Theme.tabBarTextColorSelected : Theme.tabBarTextColor
            }

            Button {
              id: closeButton
              anchors.right: parent.right
              anchors.verticalCenter: parent.verticalCenter
              anchors.rightMargin: 4
              height: 16
              width: 16
              iconSource: "qrc:/images/icon_delete.png"

              style: ButtonStyle {
                padding {
                  left: 2
                  right: 2
                  top: 2
                  bottom: 2
                }
                background: Rectangle {
                  anchors.fill: parent
                  radius: width/2
                  color: control.pressed ? "#777" : (control.hovered ? "#777" : "transparent")
                }
                label: Image {
                  source: control.iconSource
                  fillMode: Image.PreserveAspectFit
                }
              }
              onClicked: removeactivity(internal.activityList[styleData.index].uid)
            }
          }
        }
      }
    }

    Component {
      id: separator
      Rectangle {
        implicitWidth: 1
        implicitHeight: 1
        color: Theme.windowSeparatorColor
      }
    }

    Component {
      id: activityRect
      Rectangle {
        color: Theme.tabBgColor
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

  Component {
    id: fileDialogComp
    FileDialog {}
  }
}
