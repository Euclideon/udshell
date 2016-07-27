import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import Platform 0.1

Item {
  id: consoleWin

  property var epTypeDesc: { "id": "ui.Console", "super": "ep.UIComponent" }
  property var tabs: []

  visible: false
  enabled: false
  anchors.fill: parent

  function togglevisible()
  {
    if(!consoleWin.visible)
    {
      consoleWin.enabled = true;

      var tab = tv.getTab(tv.currentIndex);
      EPKernel.focus.pushActiveFocusItem();
      if(tab.item.consolecomp.get("hasinput"))
        tab.item.consoleIn.forceActiveFocus();
    }
    else
    {
      filterWin.close();
      consoleWin.enabled = false;
      EPKernel.focus.restoreFocus();
    }

    consoleWin.visible = !consoleWin.visible;
  }

  Component.onCompleted: {
    // Console Tab
    var tab1 = tv.addTab("Shell", consoleTab);
    tab1.active = true;
    tab1.item.consolecomp = EPKernel.createComponent("ep.Console", {"title" : "Shell", "setOutputFunc" : tab1.item.setOutText, "appendOutputFunc" : tab1.item.appendOutText, "hasInput" : true, "inputFunc" : function(str) { EPKernel.exec(str); }, "historyFileName" : "console.history", "outputLog" : false});
    var lua = EPKernel.getLua();
    tab1.item.consolecomp.call("addbroadcaster", lua.get("outputbroadcaster"));
    tabs.push(tab1);

    // Log Tab
    var tab2 = tv.addTab("Log", consoleTab);
    tab2.active = true;
    tab2.item.consolecomp = EPKernel.createComponent("ep.Console", {"title" : "Log", "setOutputFunc" : tab2.item.setOutText, "appendOutputFunc" : tab2.item.appendOutText, "hasInput" : false, "outputLog" : true});
    tabs.push(tab2);

    // StdOut/StdErr Tab
    var tab3 = tv.addTab("StdOut/StdErr", consoleTab);
    tab3.active = true;
    tab3.item.consolecomp = EPKernel.createComponent("ep.Console", {"title" : "StdOut/StdErr", "setOutputFunc" : tab3.item.setOutText, "appendOutputFunc" : tab3.item.appendOutText, "hasInput" : false, "outputLog" : false});
    tab3.item.consolecomp.call("addbroadcaster", EPKernel.getStdOutBroadcaster());
    tab3.item.consolecomp.call("addbroadcaster", EPKernel.getStdErrBroadcaster());
    tabs.push(tab3);
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
            onOpened: {
              contentItem.consolecomp = tv.getTab(tv.currentIndex).item.consolecomp;
            }
            onClosed: {
              var tab = tv.getTab(tv.currentIndex);
              if(tab.item.consolecomp.get("hasinput"))
                tab.item.consoleIn.forceActiveFocus();
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
    id: tv

    property int overlap: 1
    property int tabHeight: 20

    tabsVisible: true
    frameVisible: false
    y: consoleHeader.y + consoleHeader.height - tabHeight - overlap

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
      id: consoleTab

      ColumnLayout {
        onConsolecompChanged: {
          consoleInRect.visible = consolecomp.get("hasinput");
          consolecomp.call("rebuildoutput");

          if(!visible)
            bFirstTimeVisible = true;
        }

        onVisibleChanged: {
          if(visible) {
            if(filterWin.visible) {
              filterWin.contentItem.consolecomp = consolecomp;
            }

            if(bFirstTimeVisible) { // Some glitch causes the scrollbars to move between when the tab is created and when it first becomes visible. This corrects it when the tab first becomes visible
              consoleOut.flickableItem.contentY = Math.max(0, consoleOut.flickableItem.contentHeight - consoleOut.flickableItem.height);
              consoleOut.flickableItem.contentX = consoleOut.flickableItem.originX;
              bFirstTimeVisible = false;
            }
          }
        }

        function appendOutText(str)
        {
          var atXBeginning = consoleOut.flickableItem.atXBeginning;

          consoleOut.append(str);

          if(atXBeginning)
            consoleOut.flickableItem.contentX = consoleOut.flickableItem.originX;
        }

        function setOutText(str)
        {
          consoleOut.cursorPosition = 0;
          consoleOut.text = str;
          consoleOut.flickableItem.contentY = Math.max(0, consoleOut.flickableItem.contentHeight - consoleOut.flickableItem.height);
          consoleOut.flickableItem.contentX = consoleOut.flickableItem.originX;
        }

        property var consolecomp
        property bool bFirstTimeVisible: false
        property alias consoleIn: consoleIn
        property alias consoleOut: consoleOut
        spacing: 0

        Rectangle {
          Layout.fillHeight: true
          Layout.fillWidth: true
          color: "black"
          opacity: 0.9
          TextArea {
            id: consoleOut
            activeFocusOnTab: false
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

        TextArea {
          id: minTextArea
          visible: false
          text: "foo"
          textMargin: consoleIn.textMargin
          font: consoleIn.font
          activeFocusOnTab: false
        }

        Rectangle {
          id: consoleInRect
          color: "black"
          visible: false
          opacity: 0.9
          Layout.preferredHeight: innerRect.height + 2
          Layout.fillWidth: true
          Rectangle {
            id: innerRect
            height: consoleIn.height + border.width * 2
            width: parent.width
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            border.color: "white"
            border.width: 1
            TextArea {
              id: consoleIn
              property int historyIndex: 0
              focus: true
              activeFocusOnTab: true
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
                if(event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                  if(event.modifiers === Qt.ShiftModifier)
                    insert(cursorPosition, "\n");
                  else {
                    if(length != 0) {
                      consolecomp.call("relayinput", text);

                      cursorPosition = 0;
                      text = "";
                      historyIndex = 0;
                    }
                  }
                  event.accepted = true;
                }
                else if(event.key === Qt.Key_Up) {
                  if(cursorRectangle.y != positionToRectangle(0).y)
                    return;

                  var historyText = text;
                  do {
                    if(Math.abs(historyIndex) >= consolecomp.get("historylength"))
                      break;

                    historyIndex--;
                    if(historyIndex < 0)
                      historyText = consolecomp.call("gethistoryline", historyIndex);
                  } while(historyText == text);

                  if(historyText != text) {
                    cursorPosition = 0;
                    text = historyText;
                    cursorPosition = text.length;
                  }

                  event.accepted = true;
                }
                else if(event.key === Qt.Key_Down) {
                  if(cursorRectangle.y != positionToRectangle(length).y)
                    return;

                  var historyText = "";
                  do {
                    if(historyIndex < 0) {
                      historyIndex++;
                      if(historyIndex < 0)
                        historyText = consolecomp.call("gethistoryline", historyIndex);
                      else
                        historyText = "";
                    }
                  } while(historyText != "" && historyText == text);

                  if(historyText != text) {
                    cursorPosition = 0;
                    text = historyText;
                    cursorPosition = text.length;
                  }
                  event.accepted = true;
                }
              }
            }
          }
        }
      }
    }
  }
}




