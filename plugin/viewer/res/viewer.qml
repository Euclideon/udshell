import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import epKernel 0.1
import epControls 0.1
import epThemes 0.1

FocusScope {
  id: viewerui
  anchors.fill: parent
  anchors.topMargin: 4
  anchors.bottomMargin: 4

  property var viewport
  property var sidebarcomp
  property var bookmarkscomp
  property var bookmarksqq
  property var resourcespanel

  signal resourcedropped(var uid, var x, var y)

  function togglebookmarkspanel() {
    toolPanelSlot.toggleVisible(bookmarkscomp.get("uihandle"));
  }

  onBookmarkscompChanged: {
    var bookmarksqq = bookmarkscomp.get("uihandle");
    toolPanelSlot.addPanel(bookmarksqq, 200);
  }

  function toggleresourcespanel() {
    toolPanelSlot.toggleVisible(resourcespanel.get("uihandle"));
  }

  onResourcespanelChanged: {
    var resourcespanelqq = resourcespanel.get("uihandle");
    toolPanelSlot.addPanel(resourcespanelqq, 300);
  }

  Component.onCompleted: {
    sidebarcomp = EPKernel.createComponent("menu", {"name" : "sidebar"});
    if (!sidebarcomp) {
      console.error("Unable to create Sidebar Component");
      return;
    }

    var resourcesPanelButton = {
      "name" : "Toggle Resources Panel",
      "type" : "button",
      "command" : "toggleresourcespanel",
      "description" : "Toggles open/closed the Resources panel",
      "image" : "qrc:/images/icon_UDS_48.png",
    };
    sidebarcomp.call("additem", "", resourcesPanelButton);

    var bookMarksButton = {
      "name" : "Toggle Bookmarks Panel",
      "type" : "button",
      "command" : "togglebookmarkspanel",
      "description" : "Toggles open/closed the Bookmarks panel",
      "image" : "qrc:/images/icon_bookmark_24.png",
    };
    sidebarcomp.call("additem", "", bookMarksButton);

    sideBar.toolbarcomp = sidebarcomp;
  }

  onViewportChanged: viewport.get("uihandle").parent = viewPanel

  // HACK: We have to set both of these because of inconsistent behaviour in TabView
  focus: true
  activeFocusOnTab: true

  onActiveFocusChanged: {
    if(activeFocus)
      viewport.get("uihandle").forceActiveFocus();
  }

  RowLayout {
    id: mainLayout
    spacing: 4
    anchors.fill: parent
    EPToolBar {
      focus: false
      id: sideBar
      vertical: true
      color: Theme.activitySideBarBgColor
      Layout.fillHeight: true
      Layout.preferredWidth: Theme.activitySideBarWidth
    }

    EPToolPanelManager {
      id: toolPanelSlot
      Layout.fillHeight: true
    }

    Item {
      Layout.fillHeight: true
      Layout.fillWidth: true
      Item {
        id: viewPanel
        anchors.fill: parent
        anchors.rightMargin: 8

        DropArea {
          anchors.fill: parent
          keys: ["text/uri-list", "RMResource"]

          onEntered: {
            if (drag.keys.indexOf("text/uri-list") > -1) {
              if (drag.hasText && (drag.proposedAction == Qt.MoveAction || drag.proposedAction == Qt.CopyAction)) {
                var resourceManager = EPKernel.findComponent("resourcemanager");
                var exts = resourceManager.get("extensions");
                var udDSExts = exts.uddatasource; // TODO Currently the scene only supports UDDataSources. Add GeomSource extensions, when they become available. Also ImageSource and maybe others?

                var urls = drag.text.trim().split("\n");

                for(var i = 0; i < urls.length; i++) {
                  var dropFileExt = urls[i].substr((~-urls[i].lastIndexOf(".") >>> 0) + 1).toLowerCase();

                  if(udDSExts.indexOf(dropFileExt) > -1)
                    return;
                }
              }
            }

            drag.accepted = false;
          }

          onDropped: {
            if (drop.keys.indexOf("text/uri-list") > -1) {
              if (drop.hasText) {
                if (drop.proposedAction == Qt.MoveAction || drop.proposedAction == Qt.CopyAction) {
                  var urls = drop.text.trim().split("\n");

                  var resourceManager = EPKernel.findComponent("resourcemanager");

                  for(var i = 0; i < urls.length; i++) {
                    var dataSource = resourceManager.call("loadresourcesfromfile", {"src" : urls[i], "useStreamer" : true });

                    if(dataSource) {
                      var resource = dataSource.call("getresourcebyvariant", 0);
                      resourcedropped(resource.get("uid"), drop.x, drop.y);
                    }
                  }

                  drop.accept();
                }
              }
            }
            else if(drop.keys.indexOf("RMResource") > -1)
              resourcedropped(drop.source.payload.uid, drop.x, drop.y);
          }
        }
      }
    }
  }
}
