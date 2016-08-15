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
  property var simplecamera: null
  property var view: null
  property var activityselector
  property var activitylist: []
  property var activityuilist: []
  property var tablist: [] // Store a strong reference to the tabs to stop TabView from garbage collecting them

  property var fileDialog

  signal activitychanged(string uid)

  signal newprojectsignal(string path)
  signal openprojectsignal(string path)
  signal saveprojectsignal()
  signal saveprojectassignal(string path)
  signal newactivitysignal(string id)

  onMessageboxcompChanged: {
    messagebox = messageboxcomp.uiHandle;
    messagebox.parent = topLevel;
  }

  onUiconsoleChanged: {
    var uiconsoleqq = uiconsole.uiHandle;
    uiconsoleqq.parent = consolePanel;
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

  function showhideconsolepanel() {
    if(uiconsole) {
       var uiconsoleqq = uiconsole.uiHandle;
       uiconsoleqq.togglevisible();
    }
  }

  Component.onCompleted: {
    var commandManager = EPKernel.getCommandManager();
    commandManager.registerCommand("newproject", newproject, "", "", "Ctrl+N");
    commandManager.registerCommand("openproject", openproject, "", "", "Ctrl+O");

    commandManager.registerCommand("saveproject", saveproject, "", "", "Ctrl+S");
    commandManager.registerCommand("saveprojectas", saveprojectas, "", "", "F12");
    commandManager.registerCommand("newactivity", newactivity, "", "", "Ctrl+A");

    commandManager.registerCommand("showhideconsolepanel", showhideconsolepanel, "", "", "`");

    // Disable these shortcuts, they will get enabled when a project is created or opened
    commandManager.disableShortcut("saveproject");
    commandManager.disableShortcut("saveprojectas");
    commandManager.disableShortcut("newactivity");
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

  function initViewerUI(activity) {
    if(simplecamera)
      simplecamera.repositioned.disconnect(cameraupdated);

    if(view)
    {
      view.mousePositionChanged.disconnect(viewmouseupdated);
      view.enabledPickingChanged.disconnect(viewpickingenabledchanged);
      view.pickFound.disconnect(viewpickfound);
    }


    if(!activity)
    {
      simplecamera = null;
      view = null;
      return;
    }

    simplecamera = activity.simpleCamera;
    if(simplecamera)
      simplecamera.repositioned.connect(cameraupdated);

    view = activity.view;
    if(view) {
      view.mousePositionChanged.connect(viewmouseupdated);
      view.enabledPickingChanged.connect(viewpickingenabledchanged);
      view.pickFound.connect(viewpickfound);
    }
  }

  function addactivity(activity) {
    if(activityTabs.count == 0)
      activityTabs.visible = true;

    var title = activity.uid;
    title = title.charAt(0).toUpperCase() + title.slice(1);

    var ui = activity.ui;

    var tab = activityTabs.addTab(title);
    tablist.push(tab);
    activitylist.push(activity);
    activityuilist.push(ui);
    tablist.push(tab);
    ui.uiHandle.parent = tab;

    activityTabs.currentIndex = activityTabs.count - 1;
  }

  function removeactivity(uid) {
    for(var i = 0; i < activitylist.length; i++)
    {
      if(activitylist[i].uid == uid)
      {
        activitylist.splice(i, 1);
        activityuilist.splice(i, 1);
        activityTabs.removeTab(i);
        tablist.splice(i, 1);
        break;
      }
    }

    if(activityTabs.count == 0)
    {
      activityTabs.visible = false;
      activityTabs.currentIndex = -1;
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
          if(count > 0 && activitylist[currentIndex].uid != lastId)
          {
            var activityId = activitylist[currentIndex].uid;
            lastId = activityId
            activitychanged(activityId);
            var activity = activitylist[currentIndex];
            var activityqq = activityuilist[currentIndex].uiHandle;
            activityqq.visible = false; // Trigger an onVisibleChanged signal
            activityqq.visible = true;
            activityqq.forceActiveFocus();

            initViewerUI(activity);
          }
          else
          {
            lastId = "";
            activitychanged(null);

            initViewerUI(null);
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
          onClicked: uiconsole.uiHandle.togglevisible();
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
              onClicked: removeactivity(activitylist[styleData.index].uid)
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

  function newactivity() {
    activityselector.uiHandle.show(newActivitySelected);
  }

  function newActivitySelected(id) {
    newactivitysignal(id);
  }

  function newproject() {
    fileDialog = fileDialogComp.createObject(topLevel);
    fileDialog.title = "Select project location and enter a name for the new project";
    fileDialog.onAccepted.connect(newProjectAccepted);
    fileDialog.selectMultiple = false;
    fileDialog.selectExisting = false;
    fileDialog.nameFilters = ["*.epproj"];
    fileDialog.visible = true;
  }

  function newProjectAccepted() {
    fileDialog.destroy();
    newprojectsignal(fileDialog.fileUrl);
  }

  function openproject() {
    fileDialog = fileDialogComp.createObject(topLevel);
    fileDialog.title = "Select a Euclideon Platform Project file to open";
    fileDialog.onAccepted.connect(openProjectAccepted);
    fileDialog.selectMultiple = false;
    fileDialog.selectExisting = true;
    fileDialog.nameFilters = ["*.epproj"];
    fileDialog.visible = true;
  }

  function openProjectAccepted() {
    fileDialog.destroy();
    openprojectsignal(fileDialog.fileUrl);

  }

  function saveproject() {
    saveprojectsignal();
  }

  function saveprojectas() {
    fileDialog = fileDialogComp.createObject(topLevel);
    fileDialog.title = "Select project location and enter a name for the project";
    fileDialog.onAccepted.connect(saveProjectAsAccepted);
    fileDialog.selectMultiple = false;
    fileDialog.selectExisting = false;
    fileDialog.nameFilters = ["*.epproj"];
    fileDialog.visible = true;
  }

  function saveProjectAsAccepted() {
    fileDialog.destroy();
    saveprojectassignal(fileDialog.fileUrl);
  }

  Component {
    id: fileDialogComp
    FileDialog {}
  }
}
