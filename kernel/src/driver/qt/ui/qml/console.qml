import QtQuick 2.4
import QtQuick.Window 2.1
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1

Item {
  id: consoleWin
  visible: false
  enabled: false
  anchors.bottom: parent.bottom
  width: parent.width
  height: parent.height / 2

  property var consoleOut
  property var consoleIn
  property var logOut

  function togglevisible()
  {
    consoleWin.visible = !consoleWin.visible;
    if(consoleWin.visible)
    {
      consoleWin.enabled = true;
      consoleIn.forceActiveFocus();
    }
    else
    {
      filterWin.close();
      consoleWin.enabled = false;
    }
  }

  function appendconsoletext(str)
  {
    var atEnd = consoleOut.flickableItem.atYEnd;
    consoleOut.text += (consoleOut.text ? "\n" : "") + str;
    if(atEnd)
      consoleOut.flickableItem.contentY = Math.max(0, consoleOut.flickableItem.contentHeight - consoleOut.flickableItem.height);
  }

  function setconsoletext(str)
  {
    consoleOut.cursorPosition = 0;
    consoleOut.text = str;
    consoleOut.flickableItem.contentY = Math.max(0, consoleOut.flickableItem.contentHeight - consoleOut.flickableItem.height);
  }

  function appendlogtext(str)
  {
    var atEnd = logOut.flickableItem.atYEnd;
    logOut.text += (logOut.text ? "\n" : "") + str;
    if(atEnd)
      logOut.flickableItem.contentY = Math.max(0, logOut.flickableItem.contentHeight - logOut.flickableItem.height);
  }

  function setlogtext(str)
  {
    logOut.cursorPosition = 0;
    logOut.text = str;
    logOut.flickableItem.contentY = Math.max(0, logOut.flickableItem.contentHeight - logOut.flickableItem.height);
  }

  Component.onCompleted: {
    logOut = Qt.binding(function() { return logTab.item.textArea; });
    consoleOut = Qt.binding(function() { return consoleTab.item.consoleOutLoader.item.textArea; });
    consoleIn = Qt.binding(function() { return consoleTab.item.consoleInTextArea; });
  }

  Connections {
    target: thisComponent
    onCompleted: tv.splitTabs()
  }

  FontLoader { id: fixedFont; name: "Courier" }

  ColumnLayout {
    id: consoleHeader
    anchors.top: parent.top
    width: parent.width
    spacing: 0

    Rectangle {
      color: "black"
      opacity: 0.9

      Layout.fillWidth: true
      Layout.preferredHeight: consoleButtons.implicitHeight + consoleButtons.anchors.topMargin + consoleButtons.anchors.bottomMargin + 1
      Layout.minimumWidth: 300

      RowLayout {
        id: consoleButtons
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 2
        spacing: 4

        Component {
          id: consoleButtonStyle
          ButtonStyle {
            label: Text {
              font.pixelSize: 11
              text: control.text
              verticalAlignment: Text.AlignVCenter
              horizontalAlignment: Text.AlignHCenter
            }
            padding {
              left: 3; right: 3; top: 2; bottom: 2
            }
            background: Rectangle {
              border.width: control.activeFocus ? 2 : 1
              border.color: "#888"
              radius: 4
              gradient: Gradient {
                GradientStop { position: 0 ; color: control.pressed ? "#ccc" : "#eee" }
                GradientStop { position: 1 ; color: control.pressed ? "#aaa" : "#ccc" }
              }
            }
          }
        }

        Button {
          id: filterButton
          text: "Filter"
          style: consoleButtonStyle
          onClicked: {
            if(!filterWin.visible)
              filterWin.open();
            else
              filterWin.close();
          }

          Popup {
            id: filterWin
            anchors.bottom: parent.top
            anchors.right: parent.right
            anchors.bottomMargin: 5
            focus: true
            source: "consolefilter.qml"
            onClosed: consoleIn.forceActiveFocus();
          }
        }

        Button {
          id: splitButton
          text: "< >"
          style: consoleButtonStyle
          onClicked: {
            if(tv.bMerged)
            {
              tv.splitTabs();
              text = "> <";
            }
            else
            {
              tv.mergeTabs();
              text = "< >";
            }
          }
        }

        Button {
          id: closeButton
          text: "X"
          style: consoleButtonStyle
          onClicked: {
            consoleWin.enabled = false;
            consoleWin.visible = false;
          }
        }
      }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.minimumWidth: 300
        Layout.preferredHeight: 1
        color: "dimgrey"
    }
  }

  TabView {
    property int overlap: 1
    property int tabHeight: 20
    property bool bMerged: true
    property bool bFirstTimeShown: false // This is a hack to correct scrollbars

    function mergeTabs() {
      bMerged = true;
      currentIndex = 0;
      tabsVisible = false;
      y += tabHeight - overlap;
      thisComponent.set("outputsmerged", true);
    }

    function splitTabs() {
      bMerged = false;
      tabsVisible = true;
      y -= tabHeight - overlap;
      thisComponent.set("outputsmerged", false);
    }

    id: tv
    tabsVisible: false
    frameVisible: false
    y: consoleHeader.y + consoleHeader.height

    width: parent.width
    height: parent.height - y
    style: TabViewStyle {
      frameOverlap: tv.overlap
      tab: Rectangle {
        color: styleData.selected ? "steelblue" :"lightsteelblue"
        border.color: "steelblue"
        implicitWidth: Math.max(text.width + 4, 80)
        implicitHeight: tv.tabHeight
        radius: 2
        Text {
          id: text
          anchors.centerIn: parent
          text: styleData.title
          color: styleData.selected ? "white" : "black"
        }
      }
      tabBar: Rectangle { visible: false }
    }

    Component {
      id: textOut
      Rectangle {
        property alias textArea: textArea
        color: "black"
        opacity: 0.9
        TextArea {
          id: textArea
          anchors.fill: parent
          frameVisible: false
          wrapMode: TextEdit.NoWrap

          style: TextAreaStyle {
            textMargin: 4
            backgroundColor: "transparent"
            textColor: "white"
            selectionColor: "darkblue"
            font: fixedFont.name
          }
          readOnly: true
          selectByMouse: true
          onActiveFocusChanged: {
            if(!activeFocus)
              deselect();
          }
        }
      }
    }

    Tab {
      id: consoleTab
      title: "Shell"
      active: true

      ColumnLayout {
        property alias consoleInTextArea: consoleInTextArea
        property alias consoleOutLoader: consoleOutLoader
        spacing: 0

        Loader {
          id: consoleOutLoader
          Layout.fillHeight: true
          Layout.fillWidth: true
          sourceComponent: textOut
        }

        TextArea {
          id: minTextArea
          visible: false
          text: "foo"
          textMargin: consoleInTextArea.textMargin
          font: consoleInTextArea.font
        }

        Rectangle {
          color: "black"
          opacity: 0.9
          Layout.preferredHeight: innerRect.height + 1
          Layout.fillWidth: true
          Rectangle {
            id: innerRect
            height: consoleInTextArea.height + border.width * 2
            width: parent.width
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            border.color: "white"
            border.width: 1
            TextArea {
              id: consoleInTextArea
              focus: true
              width: parent.width - parent.border.width * 2
              height: Math.max(minTextArea.contentHeight, Math.min(0.33 * consoleWin.height, contentHeight)) + 2
              anchors.verticalCenter: parent.verticalCenter
              anchors.horizontalCenter: parent.horizontalCenter
              frameVisible: false
              verticalAlignment: TextEdit.AlignVCenter
              font: fixedFont.name
              style: TextAreaStyle {
                backgroundColor: "black"
                textColor: "white"
                selectionColor: "darkblue"
              }

              Keys.onPressed: {
                if(event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
                {
                  if(event.modifiers === Qt.ShiftModifier)
                    insert(cursorPosition, "\n");
                  else
                  {
                    if(length != 0)
                      thisComponent.call("relayinput", text);

                    cursorPosition = 0;
                    text = "";
                  }
                  event.accepted = true;
                }
              }
            }
          }
        }
      }
    }

    Tab {
      id: logTab
      title: "Log"
      active: true
      sourceComponent: textOut
    }
  }
}




