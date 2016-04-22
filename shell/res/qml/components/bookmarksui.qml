import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import epKernel 0.1
import epControls 0.1
import epThemes 0.1

/** To use:
  * After creating this component, you need to pass it a View component, e.g. SetProperty("view", spView);
  */

Rectangle {
  id: bookmarksui
  anchors.fill: parent
  color: Theme.toolPanelBgColor

  property var epTypeDesc: { "super": "ep.uicomponent", "id": "ui.bookmarksui" }
  property var commandManager
  property var messagebox
  property var view

  Component.onCompleted: {
    commandManager = EPKernel.getCommandManager();
    messagebox = EPKernel.findComponent("messagebox");
  }

  function createbookmark(name) {
    if(!name)
    {
      var highestID = 0;
      var pattern = /^Bookmark \d+$/;

      for(var i = 0; i < bookmarks.count; i++)
      {
        var bookmark = bookmarks.get(i);
        if(bookmark.name.match(pattern))
        {
          var id = parseInt(bookmark.name.split(' ')[1], 10);
          if(id > highestID)
            highestID = id;
        }
      }

      name = "Bookmark " + (highestID + 1);
    }

    listView.currentIndex = bookmarks.insertOrdered({"name" : name});

    return name;
  }

  EPListModel {
    id: bookmarks
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
          id: addBookmarkButton
          iconSource: "qrc:/images/icon_bookmark_addnew_24.png"
          onClicked: {
            var bmName = bookmarksui.createbookmark("");
            var spScene = view.get("scene");
            spScene.call("addbookmarkfromcamera", bmName, view.get("camera"));
          }
          Layout.preferredHeight: toolBar.height
          Layout.preferredWidth: toolBar.height
        }
        EPToolButton {
          id: deleteBookmarkButton
          iconSource: "qrc:/images/icon_delete.png"
          onClicked: {
            if(listView.currentIndex != -1) {
              var spScene = view.get("scene");
              spScene.call("removebookmark", listView.selectedItemData.name);
              bookmarks.remove(listView.currentIndex, 1);
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

      EPListView {
        id: listView
        anchors.fill: parent
        model: bookmarks
        modelDisplayItem: "name"
        onItemSelected: view.call("gotobookmark", listView.selectedItemData.name);

        function showRenameBookmarkEditBox() {
          messagebox.call("show", {
            "title": "Rename Bookmark",
            "text": "Enter new Bookmark name",
            "callback": renameBookmarkCallback,
            "type": "Edit",
            "initEditText": listView.rightClickItemData.name,
            "validator": function(text) { if(text == "") return false; return true; }
          });
        }

        function renameBookmarkCallback(retValues) {
          if(retValues.buttonLabel == "OK") {
            if(retValues.editText == "")
            {
              showRenameBookmarkEditBox();
              return;
            }
            else {
              for(var i = 0; i < bookmarks.count; i++) {
                if(bookmarks.get(i).name == retValues.editText && listView.rightClickIndex != i)
                {
                  messagebox.call("show", {
                    "title": "Error renaming Bookmark",
                    "text": "A Bookmark with that name already exists",
                    "callback": showRenameBookmarkEditBox
                  });
                  return;
                }
              }
            }

            var bm = listView.rightClickItemData;

            var spScene = view.get("scene");
            spScene.call("renamebookmark", bm.name, retValues.editText);

            bookmarks.remove(listView.rightClickIndex);
            var newIndex = bookmarks.insertOrdered({"name" : retValues.editText});
            if(listView.rightClickIndex == listView.currentIndex)
              listView.currentIndex = newIndex;
          }
        }

        menu: EPMenu {
          MenuItem {
            text: "Save location to Bookmark"
            onTriggered: {
              var spScene = view.get("scene");
              spScene.call("addbookmarkfromcamera", listView.rightClickItemData.name, view.get("camera"))
            }
          }
          MenuItem {
            text: "Rename"
            onTriggered: {
              listView.showRenameBookmarkEditBox();
            }
          }
          MenuItem {
            text: "Delete"
            onTriggered: {
              var spScene = view.get("scene");
              spScene.call("removebookmark", listView.rightClickItemData.name);
              bookmarks.remove(listView.rightClickIndex, 1);
            }
          }
        }
      }
    }
  }
}
