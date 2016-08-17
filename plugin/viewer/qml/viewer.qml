import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2
import Platform 0.1
import Platform.Controls 0.1
import Platform.Themes 0.1

FocusScope {
  id: viewerui
  anchors.fill: parent
  anchors.topMargin: 4
  anchors.bottomMargin: 4

  // HACK: We have to set both of these because of inconsistent behaviour in TabView
  focus: true
  activeFocusOnTab: true

  // Properties // ------------------------------------------------------------
  property var epTypeDesc: { "id": "viewer.MainUI", "super": "ep.UIComponent" }
  property var viewport
  property var bookmarks
  // TODO: Bug EP-66
  //property var resourcespanel

  // Events // ----------------------------------------------------------------
  signal resourceDropped(string uid, int x, int y)

  // Methods // ---------------------------------------------------------------
  function toggleBookmarksPanel()
  {
    toolPanelSlot.toggleVisible(bookmarks.uiHandle);
  }

  // Event Handlers // --------------------------------------------------------
  onBookmarksChanged: {
    toolPanelSlot.addPanel(bookmarks.uiHandle, 200);
  }

  // TODO: Bug EP-66
  //function toggleresourcespanel() {
    //toolPanelSlot.toggleVisible(resourcespanel.get("uihandle"));
  //}

  // TODO: Bug EP-66
  //onResourcespanelChanged: {
    //var resourcespanelqq = resourcespanel.get("uihandle");
    //toolPanelSlot.addPanel(resourcespanelqq, 300);
  //}

  Component.onCompleted: {
    var sidebarcomp = EPKernel.createComponent("ep.Menu", {});
    if (!sidebarcomp) {
      console.error("Unable to create Sidebar Component");
      return;
    }

    // TODO: Bug EP-66
    /*var resourcesPanelButton = {
      "name" : "Toggle Resources Panel",
      "type" : "button",
      "command" : "toggleresourcespanel",
      "description" : "Toggles open/closed the Resources panel",
      "image" : "qrc:/images/icon_UDS_48.png",
    };
    sidebarcomp.call("additem", "", resourcesPanelButton);*/

    var bookmarksButton = {
      "name" : "Toggle Bookmarks Panel",
      "type" : "button",
      "command" : "toggleBookmarksPanel",
      "description" : "Toggles open/closed the Bookmarks panel",
      "image" : "qrc:/images/icon_bookmark_24.png",
    };
    sidebarcomp.addItem("", bookmarksButton);

    sideBar.toolbarcomp = sidebarcomp;
  }

  onViewportChanged: viewport.uiHandle.parent = viewPanel

  onActiveFocusChanged: {
    if(activeFocus)
      viewport.uiHandle.forceActiveFocus();
  }

  // Item Tree // -------------------------------------------------------------
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
                var exts = resourceManager.extensions;
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
                    var dataSource = resourceManager.loadResourcesFromFile({"src" : urls[i], "useStreamer" : true });

                    if(dataSource) {
                      var resource = dataSource.getResourceByVariant(0);
                      resourceDropped(resource.uid, drop.x, drop.y);
                    }
                  }

                  drop.accept();
                }
              }
            }
            else if(drop.keys.indexOf("RMResource") > -1)
              resourceDropped(drop.source.payload.uid, drop.x, drop.y);
          }
        }
      }
    }
  }
}
