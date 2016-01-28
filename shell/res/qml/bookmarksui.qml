import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import epKernel 0.1
import "qrc:/qml/epcontrols" 0.1

/** To use:
  * After creating this component, you need to pass it a View component, e.g. SetProperty("view", spView);
  */

Rectangle {
  id: bookmarksui
  anchors.fill: parent
  color: "#444"

  property var commandManager
  property var messagebox
  property var view

  Component.onCompleted: {
    commandManager = EPKernel.GetCommandManager();
    messagebox = EPKernel.FindComponent("messagebox");
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

    listView.currentIndex = bookmarks.insertAlphabetical({"name" : name});

    return name;
  }

  ListModel {
    id: bookmarks

    function insertAlphabetical(item) {
      var i;
      for(i = 0; i < bookmarks.count; i++) {
        if(item.name.toLowerCase() < bookmarks.get(i).name.toLowerCase())
          break;
      }
      bookmarks.insert(i, item);

      return i;
    }
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 0
    Rectangle {
      color: "#555"
      Layout.fillWidth: true
      Layout.preferredHeight: 40
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
            var spScene = view.get("scene");
            spScene.call("removebookmark", bookmarks.get(listView.currentIndex).name);
            bookmarks.remove(listView.currentIndex, 1);
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
      ListView {
        id: listView
        clip: true
        property int rightClickIndex
        property bool menuShowing: false
        highlightMoveVelocity: 2000
        anchors.fill: parent
        anchors.margins: 0
        model: bookmarks
        delegate: Rectangle {
          height: text.height
          width: listView.width
          color: ((mouseArea.containsMouse && !listView.menuShowing) || (listView.menuShowing && listView.rightClickIndex == index)) && (listView.currentIndex != index) ? "blue" : "transparent"

          MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onContainsPressChanged: {
              if(pressedButtons == Qt.LeftButton) {
                listView.currentIndex = index;
                view.call("gotobookmark", bookmarks.get(listView.currentIndex).name);
              }
              else if(pressedButtons == Qt.RightButton) {
                listView.rightClickIndex = index;
                menu.popup();
              }
            }
          }

          function showRenameBookmarkEditBox() {
            messagebox.call("show", {
              "title": "Rename Bookmark",
              "text": "Enter new Bookmark name",
              "callback": renameBookmarkCallback,
              "type": "Edit",
              "initEditText": bookmarks.get(listView.rightClickIndex).name,
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

              var bm = bookmarks.get(listView.rightClickIndex);

              var spScene = view.get("scene");
              spScene.call("renamebookmark", bm.name, retValues.editText);

              bookmarks.remove(listView.rightClickIndex);
              var newIndex = bookmarks.insertAlphabetical({"name" : retValues.editText});
              if(listView.rightClickIndex == listView.currentIndex)
                listView.currentIndex = newIndex;
            }
          }

          EPMenu {
            id: menu

            on__PopupVisibleChanged: {
              listView.menuShowing = __popupVisible;
            }

            MenuItem {
              text: "Save location to Bookmark"
              onTriggered: {
                var spScene = view.get("scene");
                spScene.call("addbookmarkfromcamera", bookmarks.get(listView.rightClickIndex).name, view.get("camera"))
              }
            }
            MenuItem {
              text: "Rename"
              onTriggered: {
                showRenameBookmarkEditBox();
              }
            }
            MenuItem {
              text: "Delete"
              onTriggered: {
                var spScene = view.get("scene");
                spScene.call("removebookmark", bookmarks.get(listView.rightClickIndex).name);
                bookmarks.remove(listView.rightClickIndex, 1);
              }
            }
          }

          Text {
            id: text
            clip: true
            anchors.leftMargin: 10
            anchors.rightMargin: 5
            anchors.left: parent.left
            anchors.top: parent.top
            width: parent.width - anchors.leftMargin - anchors.rightMargin
            height: contentHeight + 15
            verticalAlignment: Text.AlignVCenter
            color: "white"
            text: name
          }
        }
        highlight: Rectangle {
          color: "#777"
          height: listView.currentItem.height
          width: listView.currentItem.width
        }
      }
    }
  }
}
