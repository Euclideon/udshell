import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import epKernel 0.1
import epControls 0.1

Rectangle {
  id: topLevel
  anchors.fill: parent
  color: "#444"

  property var uiconsole
  property var messageboxcomp
  property var messagebox
  property var menucomp
  property var toolbarcomp
  property var simplecamera: null
  property var view: null
  property var activityselecter
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
    messagebox = messageboxcomp.get("uihandle");
    messagebox.parent = topLevel;
  }

  onUiconsoleChanged: {
    var uiconsoleqq = uiconsole.get("uihandle");
    uiconsoleqq.parent = consolePanel;
  }

  onMenucompChanged: {
    menuBar.menucomp = menucomp;
  }

  onToolbarcompChanged: {
    toolBar.toolbarcomp = toolbarcomp;
  }

  onActivityselecterChanged: {
    activityselecter.get("uihandle").parent = this;
  }

  Component.onCompleted: {
    var commandManager = EPKernel.getCommandManager();
    commandManager.call("registercommand", "newproject", newproject, "", "", "Ctrl+N");
    commandManager.call("registercommand", "openproject", openproject, "", "", "Ctrl+O");

    commandManager.call("registercommand", "saveproject", saveproject, "", "", "Ctrl+S");
    commandManager.call("registercommand", "saveprojectas", saveprojectas, "", "", "F12");
    commandManager.call("registercommand", "newactivity", newactivity, "", "", "Ctrl+A");

    // Disable these shortcuts, they will get enabled when a project is created or opened
    commandManager.call("disableshortcut", "saveproject");
    commandManager.call("disableshortcut", "saveprojectas");
    commandManager.call("disableshortcut", "newactivity");
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
    // TODO Uncomment this when unsubscribe works
    /*
    if(simplecamera)
      simplecamera.unsubscribe("changed", cameraupdated);

    if(view)
    {
      view.unsubscribe("mousepositionchanged", viewmouseupdated);
      view.unsubscribe("enabledpickingchanged", viewpickingenabledchanged);
      view.unsubscribe("pickfound", viewpickfound);
    }
    */

    if(!activity)
    {
      simplecamera = null;
      view = null;
      return;
    }

    simplecamera = activity.get("simplecamera");
    if(simplecamera)
      simplecamera.subscribe("changed", cameraupdated);

    view = activity.get("view");
    if(view) {
      view.subscribe("mousepositionchanged", viewmouseupdated);
      view.subscribe("enabledpickingchanged", viewpickingenabledchanged);
      view.subscribe("pickfound", viewpickfound);
    }
  }

  function addactivity(activity) {
    if(activityTabs.count == 0)
      activityTabs.visible = true;

    var title = activity.get("uid");
    title = title.charAt(0).toUpperCase() + title.slice(1);

    var ui = activity.get("ui");

    var tab = activityTabs.addTab(title);
    tablist.push(tab);
    activitylist.push(activity);
    activityuilist.push(ui);
    tablist.push(tab);
    ui.get("uihandle").parent = tab;

    activityTabs.currentIndex = activityTabs.count - 1;
  }

  function removeactivity(uid) {
    for(var i = 0; i < activitylist.length; i++)
    {
      if(activitylist[i].get("uid") == uid)
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

    EPToolBar {
      id: toolBar
      Layout.fillWidth: true
      Layout.preferredHeight: 36
    }

    Loader { sourceComponent: separator; Layout.fillWidth: true }

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
          color: "#333"
        }

        onCurrentIndexChanged: {
          if(count > 0 && activitylist[currentIndex].get("uid") != lastId)
          {
            var activityId = activitylist[currentIndex].get("uid");
            lastId = activityId
            activitychanged(activityId);
            var activity = activitylist[currentIndex];
            var activityqq = activityuilist[currentIndex].get("uihandle");
            activityqq.visible = false; // Trigger an onVisibleChanged signal
            activityqq.visible = true;
            activityqq.forceActiveFocus();

            if (activity.get("type") == "viewer")
              initViewerUI(activity);
            else
              initViewerUI(null);
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
        implicitWidth: 1
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

  function newactivity() {
    activityselecter.call("show", newActivitySelected);
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
