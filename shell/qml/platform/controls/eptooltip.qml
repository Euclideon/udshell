import QtQuick 2.0
import QtQuick.Controls 1.1
import QtGraphicalEffects 1.0

Item {
  id: toolTipRoot
  width: toolTip.contentWidth
  height: toolTipContainer.height
  visible: false
  clip: false

  property alias text: toolTip.text
  property alias radius: content.radius
  property alias backgroundColor: content.color
  property alias textColor: toolTip.color
  property alias font: toolTip.font
  property var mouseArea
  property var target: null
  property int fadeInDelay: 750
  property int fadeOutDelay: 0

  function onMouseHover(x, y) {
    var obj = toolTipRoot.target.mapToItem(toolTipRoot.parent, x, y);
    toolTipRoot.x = obj.x;
    toolTipRoot.y = obj.y + 5;

    parent = thisComponent.parentWindow().contentItem;
  }

  function onVisibleStatus(flag) {
    if(flag) {
      if(toolTip.text != "")
        state = "showing";
    }
    else
      state = "hidden";
  }

  Component {
    id: mouseAreaComp
    MouseArea {
      signal mouserHover(int x, int y)
      signal showChanged(bool flag)

      anchors.fill: parent
      hoverEnabled: true

      onPositionChanged: { mouserHover(mouseX, mouseY); }
      onEntered: { showChanged(true); }
      onExited: { showChanged(false); }
      onClicked: {
        showChanged(false);
        if(target.clicked != undefined)
          target.clicked();
      }
    }
  }

  function qmltypeof(obj, className) { // QtObject, string -> bool
    // className plus "(" is the class instance without modification
    // className plus "_QML" is the class instance with user-defined properties
    var str = obj.toString();
    return str.indexOf(className + "(") == 0 || str.indexOf(className + "_QML") == 0;
  }

  Component.onCompleted: {
    var itemParent = target;

    for (var i = 0; i < target.children.length; ++i) {
      if (qmltypeof(target.children[i], "QQuickMouseArea")) {
        itemParent = target.children[i]
        break;
      }
    }

    mouseArea = mouseAreaComp.createObject(itemParent);
    mouseArea.mouserHover.connect(onMouseHover);
    mouseArea.showChanged.connect(onVisibleStatus);
  }

  Item {
    id: toolTipContainer
    z: toolTipRoot.z + 1
    width: content.width + (2*toolTipShadow.radius)
    height: content.height + (2*toolTipShadow.radius)

    Rectangle {
      id: content
      anchors.centerIn: parent
      width: toolTip.contentWidth + 10
      height: toolTip.contentHeight + 10
      radius: 3

      Text {
        id: toolTip
        anchors {fill: parent; margins: 5}
        wrapMode: Text.WrapAnywhere
      }
    }
  }

  DropShadow {
    id: toolTipShadow
    z: toolTipRoot.z + 1
    anchors.fill: source
    cached: true
    horizontalOffset: 4
    verticalOffset: 4
    radius: 8.0
    samples: 16
    color: "#80000000"
    smooth: true
    source: toolTipContainer
  }

  states: [
    State {
      name: "showing"
      PropertyChanges { target: toolTipRoot; visible: true }
    },
    State {
      name: "hidden"
      PropertyChanges { target: toolTipRoot; visible: false }
    }
  ]

  transitions: [
    Transition {
      to: "showing"
      NumberAnimation { target: toolTipRoot; property: "visible"; duration: fadeInDelay }
    },
    Transition {
      to: "hidden"
      NumberAnimation { target: toolTipRoot; property: "visible"; duration: fadeOutDelay }
    }
  ]
}
