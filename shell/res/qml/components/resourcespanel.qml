import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import epKernel 0.1
import epControls 0.1
import epThemes 0.1

/** To use:
  * After creating this component, you need to pass it a View component, e.g. SetProperty("view", spView);
  */

Rectangle {
  id: resourcesPanel
  anchors.fill: parent
  color: Theme.toolPanelBgColor

  property var commandManager
  property var resourceManager
  property var messagebox
  property var view
  property var resources: []
  property var fileDialog

  Component.onCompleted: {
    commandManager = EPKernel.getCommandManager();
    messagebox = EPKernel.findComponent("messagebox");

    resourceManager = EPKernel.findComponent("resourcemanager");
    var res = resourceManager.get("resourcearray");
    for(var i = 0; i < res.length; i++)
      resources.push({ "uid" : res[i].get("uid"), "type" : res[i].get("type") });

    resourcesLM.buildFromArray(resources);

    resourceManager.subscribe("added", addResources);
    resourceManager.subscribe("removed", removeResources);
    // TODO Unsubscribe on component destruction when we can do that from QML
  }

  function addResources(resArray) {
    for(var i = 0; i < resArray.length; i++)
      addResource(resArray[i]);
  }

  function addResource(res) {
    var item = { "uid" : res.get("uid"), "type" : res.get("type") };

    resources.push(item);
    resourcesLM.insertOrdered(item);
  }

  function removeResources(resArray) {
    for(var i = 0; i < resArray.length; i++)
      removeResource(resArray[i]);
  }

  function removeResource(res) {
    var uid = res.get("uid");
    resourcesLM.remove(resourcesLM.find("uid", uid));
    for(var i = 0; i < resources.length; i++) {
      if(resources[i].uid == uid) {
        resources.splice(i, 1);
        break;
      }
    }
  }

  function openresources() {
    var exts = resourceManager.get("extensions");
    var filters = [];
    filters.push("All Files (*)");
    for(var key in exts) {
      var filter = "";
      if(key == "imagesource")
        filter = "Image files";
      else if(key == "uddatasource")
        filter = "UD data files";
      else if(key == "geomsource")
        filter = "Geometric data files";
      else
        filter = key;
      filter += " (";
      var compExts = exts[key];
      for(var i = 0; i < compExts.length; i++)
        filter += "*" + compExts[i] + " ";
      filter = filter.slice(0, -1);
      filter += ")";
      filters.push(filter);
    }

    fileDialog = fileDialogComp.createObject(resourcesPanel);
    fileDialog.title = "Select resource files to open";
    fileDialog.onAccepted.connect(openResourcesAccepted);
    fileDialog.selectMultiple = true;
    fileDialog.selectExisting = true;
    fileDialog.nameFilters = filters;
    fileDialog.selectedNameFilter = filters[0];
    fileDialog.visible = true;
  }

  function openResourcesAccepted() {
    for(var i = 0; i < fileDialog.fileUrls.length; i++) {
      var path = fileDialog.fileUrls[i].toString();
      resourceManager.call("loadresourcesfromfile", {"src" : path, "useStreamer" : true });
    }

    fileDialog.destroy();
  }

  Component {
    id: fileDialogComp
    EPFileDialog {}
  }

  EPListModel {
    id: resourcesLM
    sortColumnName: "uid"
    order: Qt.AscendingOrder
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 0

    Rectangle {
      color: Theme.toolPanelToolBarBgColor
      Layout.fillWidth: true
      Layout.preferredHeight: Theme.toolPanelToolBarSize
      id: toolBar
      RowLayout {
        spacing: 0
        EPToolButton {
          id: addResourceButton
          iconSource: "qrc:/images/icon_UDS_add_24.png"
          tooltip: "Open files and add their resources to the Resource Manaager"
          onClicked: {
            openresources();
          }
          Layout.preferredHeight: toolBar.height
          Layout.preferredWidth: toolBar.height
        }
        EPToolButton {
          id: deleteResourceButton
          iconSource: "qrc:/images/icon_delete.png"
          tooltip: "Unload the selected Resource"
          onClicked: {
            if(tableView.currentRow != -1) {
              var res = resourceManager.call("getresource", tableView.selectedItemData.uid);
              if(!res.isNull())
                resourceManager.call("removeresource", res);
            }
          }
          Layout.preferredHeight: toolBar.height
          Layout.preferredWidth: toolBar.height
        }
        Item { Layout.fillWidth: true }
      }
    }

    Item {
      Layout.fillWidth: true
      Layout.fillHeight: true

      DropArea {
        anchors.fill: parent
        keys: ["text/uri-list"]

        onDropped: {
          if (drop.hasText) {
            if (drop.proposedAction == Qt.MoveAction || drop.proposedAction == Qt.CopyAction) {
              var urls = drop.text.trim().split("\n");
              for(var i = 0; i < urls.length; i++)
                resourceManager.call("loadresourcesfromfile", {"src" : urls[i]});

              drop.acceptProposedAction();
            }
          }
        }
      }

      EPTableView {
        id: tableView
        anchors.fill: parent
        model: resourcesLM
        alternatingRowColors: false
        dragColumn: "uid"
        dragKeys: ["RMResource"]

        TableViewColumn {
          role: "uid"
          title: "ID"
          width: (tableView.width - 2) / 2
        }
        TableViewColumn {
          role: "type"
          title: "Type"
          width: (tableView.width - 2) / 2
        }

        menu: EPMenu {
          MenuItem {
            text: "Delete"
            onTriggered: {
              var res = resourceManager.call("getresource", tableView.rightClickItemData.uid);
              if(!res.isNull())
                resourceManager.call("removeresource", res);
            }
          }
        }
      }
    }

    Rectangle {
      id: filterPanel
      color: "#555"
      Layout.fillWidth: true
      Layout.preferredHeight: filterLayout.implicitHeight + filterLayout.anchors.topMargin + filterLayout.anchors.bottomMargin
      ColumnLayout {
        id: filterLayout
        anchors.fill: parent
        anchors.margins: 8
        RowLayout {
          id: filterUIDLayout
          Layout.alignment: Qt.AlignRight
          spacing: 6
          Text {
            id: filterUIDLabel
            text: "Filter by ID:"
            color: "white"
            Layout.alignment: Qt.AlignRight
          }
          TextField {
            id: filterUIDTextField
            style: filterTextFieldStyle
            Layout.preferredWidth: filterLayout.width - filterTypeLabel.implicitWidth - filterUIDLayout.spacing

            onTextChanged: {
              resourcesLM.setFilter("uid", text);
              resourcesLM.buildFromArray(resourcesPanel.resources);
            }
          }
        }

        RowLayout {
          id: filterTypeLayout
          Layout.alignment: Qt.AlignRight
          spacing: 6
          Text {
            id: filterTypeLabel
            text: "Filter by type:"
            color: "white"
            Layout.alignment: Qt.AlignRight
          }
          TextField {
            id: filterTypeTextField
            style: filterTextFieldStyle
            Layout.preferredWidth: filterLayout.width - filterTypeLabel.implicitWidth - filterTypeLayout.spacing

            onTextChanged: {
               resourcesLM.setFilter("type", text);
               resourcesLM.buildFromArray(resourcesPanel.resources);
            }
          }
        }
      }

      Component {
        id: filterTextFieldStyle
        TextFieldStyle {
          textColor: Theme.toolPanelTextFieldTextColor
          background: Rectangle {
            color: Theme.toolPanelTextFieldBgColor
            radius: Theme.toolPanelTextFieldRadius
            implicitWidth: 100
            implicitHeight: 24
            border.color: Theme.toolPanelTextFieldBorderColor
            border.width: 1
          }
        }
      }
    }
  }
}
