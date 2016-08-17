import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.2

Item
{
  // Properties // ------------------------------------------------------------
  property var epTypeDesc: { "id": "ui.MessageBox", "super": "ep.UIComponent" }

  // Methods // ---------------------------------------------------------------
  function show(params)
  {
    if (params.title != undefined)
      messageBox.title = params.title;
    else
      messageBox.title = "";

    internal.type = "Message";
    if (params.type != undefined) {
      for(var i = 0; i < internal.mbTypes.length; i++) {
        if (internal.mbTypes[i].toUpperCase() == params.type.toUpperCase()) {
          internal.type = internal.mbTypes[i];
          break;
        }
      }
    }

    if(params.text != undefined)
      message.text = params.text;
    else
      message.text = "";

    if(params.buttons != undefined)
      internal.buttons = params.buttons;
    else {
      if (internal.type == "Message")
        internal.buttons = ["OK"];
      else
        internal.buttons = ["OK", "Cancel"];
    }

    internal.callback = params.callback

    internal.iconType = "None";
    if(params.iconType != undefined) {
      for (var key in internal.icons) {
        if (key.toUpperCase() == params.iconType.toUpperCase()) {
          internal.iconType = key;
          break;
        }
      }
    }

    if (internal.type == "Edit") {
      internal.validator = params.validator;

      if(params.initEditText != undefined) {
        loader.item.editText = params.initEditText;
        loader.item.edit.selectAll();
      }

      internal.validate(loader.item.editText);
    }

    if (internal.icons[internal.iconType] != undefined) {
      icon.source = internal.icons[internal.iconType];
      icon.visible = true
    }
    else
      icon.visible = false;

    messageBox.visible = true;
  }

  // Internal // --------------------------------------------------------------
  QtObject
  {
    id: internal

    // MessageBox types: "Message" or "Edit"
    // Icon types: "None", "About", "Error" or "Warning"
    property string type: "Message"
    property string iconType: "None"
    property var callback
    property var validator
    property string editText: type == "Edit" ? loader.item.editText : ""
    property var buttons: []

    readonly property var mbTypes: ["Message", "Edit"]
    readonly property var icons: {
      "About": "qrc:/images/icon_about_16.png",
      "Error": "qrc:/images/icon_error_16.png",
      "Warning": "qrc:/images/icon_warning_16.png"
    }

    function validate(text)
    {
      if (internal.validator && !internal.validator(text)) {
        for(var i = 0; i < buttonRepeater.count - 1; i++)
          buttonRepeater.itemAt(i).enabled = false;
      }
      else {
        for(var i = 0; i < buttonRepeater.count - 1; i++)
          buttonRepeater.itemAt(i).enabled = true;
      }
    }
  }

  // Item Tree // -------------------------------------------------------------
  Window {
    id: messageBox

    width: layout.implicitWidth + layout.anchors.leftMargin + layout.anchors.rightMargin
    height: layout.implicitHeight + layout.anchors.topMargin + layout.anchors.bottomMargin
    flags: Qt.Window | Qt.WindowSystemMenuHint | Qt.WindowTitleHint | Qt.WindowCloseButtonHint
    modality: Qt.WindowModal

    // TODO The lines below were added to stop the window from being resizable, but they prevent the window from being intentionally resized by the program. Work out another solution
    //minimumWidth: width
    //minimumHeight: height
    //maximumWidth: width
    //maximumHeight: height

    onClosing: {
      messageBox.hide();

      var returnMap = {};
      returnMap["buttonIndex"] = -1;
      returnMap["buttonLabel"] = "close";
      if (internal.type == mbTypeEdit)
        returnMap["editText"] = "";

      if (internal.callback)
        internal.callback(returnMap);
    }

    ColumnLayout {
      id: layout
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.margins: 12
      spacing: 10

      Keys.onPressed: {
        if(event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
        {
          buttonRepeater.itemAt(0).clicked();
          event.accepted = true;
        }
        else if(event.key === Qt.Key_Escape)
        {
          messageBox.close();
          event.accepted = true;
        }
      }

      RowLayout {
        id: messagePanel
        spacing: 15
        Layout.preferredHeight: Math.max(32, implicitHeight);
        Layout.preferredWidth: Math.max(300, implicitWidth);

        Image {
          id: icon
          Layout.preferredHeight: 32
          Layout.preferredWidth: 32
          fillMode: Image.PreserveAspectFit;
        }
        Text {
          id: message
          Layout.fillWidth: true
        }
      }

      Loader {
        id: loader
        Layout.fillWidth: true
        visible: internal.type == "Edit" ? true : false
        sourceComponent: internal.type == "Edit" ? editComponent : null

        Component {
          id: editComponent
          ColumnLayout {
            property alias editText: edit.text
            property alias edit: edit

            TextArea {
              id: minTextArea
              visible: false
              text: "foo"
              textMargin: edit.textMargin
              font: edit.font
            }

            TextArea {
              id: edit
              Layout.fillWidth: true
              implicitHeight: Math.max(minTextArea.contentHeight, Math.min(300, contentHeight)) + 2

              Component.onCompleted: {
                forceActiveFocus();
              }

              onTextChanged: {
                internal.validate(text);
              }

              Keys.onPressed: {
                if(event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
                {
                  if(event.modifiers === Qt.ShiftModifier)
                    insert(cursorPosition, "\n");
                  else {
                    if(buttonRepeater.itemAt(0).enabled)
                      buttonRepeater.itemAt(0).clicked();
                  }
                  event.accepted = true;
                }
              }
            }
          }
        }
      }

      RowLayout {
        id: buttonPanel
        Layout.preferredHeight: 30

        Repeater {
          id: buttonRepeater
          model: internal.buttons
          Button {
            text: modelData
            onClicked: {
              messageBox.hide();
              var returnMap = {};
              returnMap["buttonIndex"] = index;
              returnMap["buttonLabel"] = modelData;
              if (internal.type == "Edit")
                returnMap["editText"] = internal.editText;

              if (internal.callback)
                internal.callback(returnMap);
            }
          }
        }
      }
    }
  }
}


