import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import epKernel 0.1
import "qrc:/qml/epcontrols" 0.1

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
      Layout.fillHeight: true
      Layout.preferredWidth: 44
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
      }
    }
  }
}
