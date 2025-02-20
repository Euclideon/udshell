import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import Platform 0.1
import Platform.Controls 0.1
import Platform.Themes 0.1

TableView {
  property var menu: null
  property int rightClickIndex: 0
  property var selectedItemData
  property var rightClickItemData
  property string dragColumn
  property var dragKeys: []
  property bool menuShowing: (menu ? menu.__popupVisible : false)

  signal itemSelected();

  onCurrentRowChanged: {
    if(currentRow != -1)
      selectedItemData = model.get(currentRow);
    else
      selectedItemData = undefined;
  }
  onMenuShowingChanged: if(menuShowing) rightClickItemData = model.get(rightClickIndex);

  Binding { target: menu; property: "parent"; value: tableView }

  id: tableView
  clip: true
  backgroundVisible: model.count > 0 ? true : false
  sortIndicatorVisible: true
  property int textHeight

  onSortIndicatorColumnChanged: {
    model.sortColumnName = getColumn(sortIndicatorColumn).role;
    model.order = sortIndicatorOrder;
    model.sort();
  }

  onSortIndicatorOrderChanged: {
    model.order = sortIndicatorOrder;
    model.sort();
  }

  onRowCountChanged: {
    selection.clear();
    currentRow = -1;
  }

  style: TableViewStyle {
    backgroundColor: Theme.tableViewBgColor
    frame: Item {}
  }

  itemDelegate: Item {
    Text {
      id: text
      clip: true
      width: parent.width
      anchors.leftMargin: 10
      anchors.rightMargin: 10
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      verticalAlignment: Text.AlignVCenter
      color: "white"
      text: styleData.value

      Component.onCompleted: tableView.textHeight = implicitHeight
    }
  }

  rowDelegate: Rectangle {
    height: tableView.textHeight * 1.3
    width: tableView.width
    color: styleData.selected ? Theme.tableViewItemBgColorSelected : ((styleData.row < tableView.rowCount) && ((mouseArea.containsMouse && !tableView.menuShowing) || (tableView.menuShowing && tableView.rightClickIndex == styleData.row)) ? Theme.tableViewItemBgColorHovered : (styleData.alternate ? Theme.tableViewItemBgColorAlternate : Theme.tableViewItemBgColor))

    MouseArea {
      id: mouseArea
      property bool bDragging: false
      property bool bLeftPressed: false
      property var dragItem
      property double mouseXStart;
      property double mouseYStart;
      anchors.fill: parent
      hoverEnabled: true
      acceptedButtons: Qt.LeftButton | Qt.RightButton

      onPositionChanged: {
        if(bLeftPressed && !bDragging && dragColumn != undefined && !bDragging &&
            (Math.abs(mouseX - mouseXStart) > 10 || Math.abs(mouseY - mouseYStart) > 10)) {

          bDragging = true;
          dragItem = dragItemComp.createObject(EP.parentWindow(thisComponent.uihandle));
          dragItem.text = model.get(styleData.row)[dragColumn];
          dragItem.payload = model.get(styleData.row);
          var winXY = mapToItem(EP.parentWindow(thisComponent.uihandle).contentItem, mouse.x - dragItem.width / 2, mouse.y - dragItem.height / 2);
          dragItem.x = winXY.x;
          dragItem.y = winXY.y;
          dragItem.Drag.active = Qt.binding(function() { return mouseArea.drag.active; });
          dragItem.Drag.hotSpot.x = dragItem.width / 2;
          dragItem.Drag.hotSpot.y = dragItem.height / 2;
          dragItem.Drag.keys = dragKeys;
          mouseArea.drag.target = dragItem;
          mouseArea.drag.threshold = 0;
        }
      }

      onReleased: {
        bLeftPressed = false;

        if(bDragging) {
          dragItem.Drag.drop();

          bDragging = false;
          mouseArea.drag.target = null;
          dragItem.destroy();
        }
      }

      onContainsPressChanged: {
        if(styleData.row == undefined || styleData.row >= tableView.rowCount)
          return;

        if(pressedButtons == Qt.LeftButton) {
          mouseXStart = mouseX;
          mouseYStart = mouseY;
          bLeftPressed = true;

          tableView.selection.clear();
          tableView.selection.select(styleData.row);
          tableView.currentRow = styleData.row;

          itemSelected();
        }
        else if(pressedButtons == Qt.RightButton) {
          tableView.rightClickIndex = styleData.row;
          if(menu)
            menu.popup();
        }
      }
    }
  }

  headerDelegate: Rectangle {
    height: textItem.implicitHeight * Theme.tableViewHeaderHeightMultiplier
    width: textItem.implicitWidth
    color: Theme.tableViewHeaderBgColor
    Text {
      id: textItem
      anchors.verticalCenter: parent.verticalCenter
      anchors.left: parent.left
      verticalAlignment: Text.AlignVCenter
      horizontalAlignment: styleData.textAlignment
      anchors.leftMargin: 10
      text: styleData.value
      elide: Text.ElideRight
      color: Theme.tableViewHeaderTextColor
      renderType: Text.NativeRendering
    }

    Text {
      id: sortIndicator
      anchors.verticalCenter: parent.verticalCenter
      anchors.right: parent.right
      font.pixelSize: Theme.tableViewHeaderSortIndicatorHeight
      anchors.rightMargin: 6
      visible: tableView.sortIndicatorVisible && styleData.column === tableView.sortIndicatorColumn
      text: tableView.sortIndicatorOrder === Qt.AscendingOrder ? "\u25b2" : "\u25bc"
      color: Theme.tableViewHeaderSortIndicatorColor
    }

    Rectangle {
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      anchors.bottomMargin: 1
      anchors.topMargin: 1
      width: 1
      color: Theme.tableViewHeaderSeparatorColor
    }
  }

  Component {
    id: dragItemComp
    Rectangle {
      property alias text: textItem.text
      property var payload
      id: rect
      color: Theme.dragTileBgColor
      border.color: Theme.dragTileBorderColor
      border.width: Theme.dragTileBorderWidth
      width: textItem.implicitWidth + (height - textItem.implicitHeight)
      height: textItem.implicitHeight * Theme.dragTileHeightMultiplier

      Text {
        id: textItem
        anchors.centerIn: parent
        color: Theme.dragTileTextColor
      }
    }
  }
}

