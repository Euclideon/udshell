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
    for(var i = 0; i < res.length; i++) {
      addResourceHelper(res[i]);
    }

    resourcesLM.buildFromArray(resources);

    resourceManager.subscribe("added", addResources);
    resourceManager.subscribe("removed", removeResources);
    // TODO Unsubscribe on component destruction when we can do that from QML
  }

  function addResources(resArray) {
    for(var i = 0; i < resArray.length; i++)
      addResource(resArray[i]);
  }

  function createResourceName(res) {
    var meta = res.get("metadata");
    var url = meta.call("get", "url");

    if(!url)
      return res.get("uid");

    var name = url.replace(/^.*[\\\/]/, ''); // get filename from path
    var name = name.replace(/\.[^/.]+$/, ''); // remove extension from filename

    var dupFound = false;
    for(var i = 0; i < resources.length; i++) {
      if(resources[i].name == name) {
        dupFound = true;
        break;
      }
    }

    if(!dupFound)
      return name;

    var highestID = 0;
    var pattern = new RegExp("^" + name + " (\d+)$");

    for(var i = 0; i < resources.length; i++)
    {
      var r = resources[i];
      if(r.name.match(pattern))
      {
        var id = parseInt(r.name.split(' ')[1], 10);
        if(id > highestID)
          highestID = id;
      }
    }

    name = name + " (" + (highestID + 1) + ")";

    return name;
  }

  function addResourceHelper(res) {
    var name = createResourceName(res);
    var item = { "uid" : res.get("uid"), "name" : name, "type" : res.get("type") };

    resources.push(item);

    return item;
  }

  function addResource(res) {
    var item = addResourceHelper(res);
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
    sortColumnName: "name"
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

        onEntered: {
          if (drag.hasText && (drag.proposedAction == Qt.MoveAction || drag.proposedAction == Qt.CopyAction)) {
            var resourceManager = EPKernel.findComponent("resourcemanager");
            var exts = resourceManager.get("extensions");

            var urls = drag.text.trim().split("\n");

            for(var i = 0; i < urls.length; i++) {
              var dropFileExt = urls[i].substr((~-urls[i].lastIndexOf(".") >>> 0) + 1).toLowerCase();

              for(var i = 0; i < exts.length; i++) {
                var dsExts = exts[i];
                if(dsExts.indexOf(dropFileExt) > -1)
                  return;
              }
            }
          }

          //drag.accepted = false; // TODO there is a bug which prevents us from refusing an onEntered event, which has been fixed in Qt 5.6. Uncomment this after switching to 5.6.
        }

        onDropped: {
          if (drop.hasText && (drop.proposedAction == Qt.MoveAction || drop.proposedAction == Qt.CopyAction)) {
            var urls = drop.text.trim().split("\n");
            for(var i = 0; i < urls.length; i++)
              resourceManager.call("loadresourcesfromfile", {"src" : urls[i]});

            drop.acceptProposedAction();
          }
        }
      }

      EPTableView {
        id: tableView
        anchors.fill: parent
        model: resourcesLM
        alternatingRowColors: false
        dragColumn: "name"
        dragKeys: ["RMResource"]

        TableViewColumn {
          role: "name"
          title: "Name"
          width: (tableView.width - 2) * (3/5)
        }
        TableViewColumn {
          role: "type"
          title: "Type"
          width: (tableView.width - 2) * (2/5)
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
          id: filterNameLayout
          Layout.alignment: Qt.AlignRight
          spacing: 6
          Text {
            id: filterNameLabel
            text: "Filter by name:"
            color: "white"
            Layout.alignment: Qt.AlignRight
          }
          TextField {
            id: filterNameTextField
            style: filterTextFieldStyle
            Layout.preferredWidth: filterLayout.width - filterNameLabel.implicitWidth - filterNameLayout.spacing

            onTextChanged: {
              resourcesLM.setFilter("name", text);
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
            Layout.preferredWidth: filterLayout.width - filterNameLabel.implicitWidth - filterTypeLayout.spacing

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
