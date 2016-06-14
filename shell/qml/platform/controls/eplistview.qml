import QtQuick 2.6
import QtQuick.Controls 1.5
import Platform.Themes 0.1

ListView {
  property var menu: null
  property int rightClickIndex: 0
  property var selectedItemData
  property var rightClickItemData
  property bool menuShowing: (menu ? menu.__popupVisible : false)
  property var modelDisplayItem
  property bool alternatingRowColors: false

  signal itemSelected();

  onCurrentItemChanged: selectedItemData = model.get(currentIndex);
  onMenuShowingChanged: if(menuShowing) rightClickItemData = model.get(rightClickIndex);

  Binding { target: menu; property: "parent"; value: listView }

  id: listView
  clip: true
  highlightMoveVelocity: Theme.listViewHighlightMoveVelocity
  delegate: Rectangle {
    height: text.height
    width: listView.width
    color: listView.currentIndex == index ? "transparent" : ((mouseArea.containsMouse && !listView.menuShowing) || (listView.menuShowing && listView.rightClickIndex == index)) ? Theme.listViewItemBgColorHovered : (alternatingRowColors ? (index % 2 == 0 ? Theme.listViewItemBgColor : Theme.listViewItemBgColorAlternate) : Theme.listViewItemBgColor)

    MouseArea {
      id: mouseArea
      anchors.fill: parent
      hoverEnabled: true
      acceptedButtons: Qt.LeftButton | Qt.RightButton

      onContainsPressChanged: {
        if(pressedButtons == Qt.LeftButton) {
          listView.currentIndex = index;
          itemSelected();
        }
        else if(pressedButtons == Qt.RightButton) {
          listView.rightClickIndex = index;
          if(menu)
            menu.popup();
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
        color: Theme.listViewItemTextColor
        textFormat: Text.RichText
        text: modelDisplayItem != undefined ? model[modelDisplayItem] : ""
      }
    }
  }
  highlight: Rectangle {
    color: Theme.listViewItemBgColorSelected
    height: listView.currentItem.height
    width: listView.currentItem.width
  }
}

