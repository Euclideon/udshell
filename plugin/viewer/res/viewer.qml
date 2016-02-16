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

  function togglebookmarkspanel() {
    bookmarksPanel.toggleVisible();
  }

  onBookmarkscompChanged: {
    bookmarksqq = bookmarkscomp.get("uihandle");
    bookmarksqq.parent = bookmarksPanel;
  }

  Component.onCompleted: {
    sidebarcomp = EPKernel.createComponent("menu", {"name" : "sidebar"});
    if (!sidebarcomp) {
      console.error("Unable to create Sidebar Component");
      return;
    }

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
    Item {
      id: bookmarksPanel
      visible: true
      Layout.preferredWidth: 200
      Layout.fillHeight: true

      function toggleVisible() {
        visible = !visible;
      }

      // HACK -- bookmarks panel doesn't initialize property when invisible
      Component.onCompleted: {
        visible = false
      }
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
