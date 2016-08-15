import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import Platform 0.1
import Platform.Controls 0.1
import Platform.Themes 0.1

/** To use:
  * After creating this component, you need to pass it a View component, e.g. SetProperty("view", spView);
  */

Rectangle {
  id: bookmarksui
  anchors.fill: parent
  color: Theme.toolPanelBgColor

  property var epTypeDesc: { "id": "ui.BookmarksUI", "super": "ep.UIComponent" }
  property var view

  function createbookmark(name)
  {
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

  Component.onCompleted: {
    internal.messageBox = EPKernel.findComponent("messagebox");
  }

  QtObject
  {
    id: internal
    property var messageBox
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
            var spScene = view.scene;
            spScene.addBookmarkFromCamera(bmName, view.camera);
          }
          Layout.preferredHeight: toolBar.height
          Layout.preferredWidth: toolBar.height
        }
        EPToolButton {
          id: deleteBookmarkButton
          iconSource: "qrc:/images/icon_delete.png"
          onClicked: {
            if(listView.currentIndex != -1) {
              var spScene = view.scene;
              spScene.removeBookmark(listView.selectedItemData.name);
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
        onItemSelected: view.gotoBookmark(listView.selectedItemData.name);

        function showRenameBookmarkEditBox() {
          internal.messageBox.show( {
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
                  internal.messageBox.show( {
                    "title": "Error renaming Bookmark",
                    "text": "A Bookmark with that name already exists",
                    "callback": showRenameBookmarkEditBox
                  });
                  return;
                }
              }
            }

            var bm = listView.rightClickItemData;

            var spScene = view.scene;
            spScene.renameBookmark(bm.name, retValues.editText);

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
              var spScene = view.scene;
              spScene.addBookmarkFromCamera(listView.rightClickItemData.name, view.camera);
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
              var spScene = view.scene;
              spScene.removeBookmark(listView.rightClickItemData.name);
              bookmarks.remove(listView.rightClickIndex, 1);
            }
          }
        }
      }
    }
  }
}
