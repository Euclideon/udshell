import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1

ColumnLayout {
  id: filterWindow
  spacing: 10
  anchors.top: parent.top
  anchors.left: parent.left
  anchors.margins: 12

  RowLayout {
    id: filterText
    Layout.alignment: Qt.AlignRight
    spacing: 6

    Text {
      Layout.alignment: Qt.AlignRight
      text: "Filter Text:"
    }
    TextField {
      id: ftTextField
      Layout.preferredWidth: 180

      onTextChanged: thisComponent.set("filtertext", ftTextField.text);
      onVisibleChanged: if(visible) ftTextField.text = thisComponent.get("filtertext");
    }
  }

  RowLayout {
    id: filterComponents
    Layout.alignment: Qt.AlignRight
    spacing: 6

    Text {
      Layout.alignment: Qt.AlignRight
      text: "Filter Log Components:"
    }
    TextField {
      id: fcTextField
      Layout.preferredWidth: 180

      onTextChanged: thisComponent.set("filtercomponents", fcTextField.text);
      onVisibleChanged: if(visible) fcTextField.text = thisComponent.get("filtercomponents");
    }
  }

  GroupBox {
    title: "Filter Log Levels"
    GridLayout {
      rowSpacing: 4
      columnSpacing: 30
      columns: 2
      anchors.fill: parent
      anchors.margins: 3

      Repeater {
        model: ["Error", "Info", "Warning", "Script", "Debug", "Trace"]
        RowLayout {
          property alias filterCatCB: filterCatCB
          Layout.preferredWidth: 120

          function updateLevelFilterUI()
          {
            var level = thisComponent.call("getfilterlevel", modelData);
            if(level != -1)
            {
              filterCatCB.checked = true;
              filterLevelField.text = level;
            }
            else
              filterCatCB.checked = false;
          }

          function updateLevelFilters()
          {
            var level;
            if(filterCatCB.checked && filterLevelField.text.length != 0)
              level = parseInt(filterLevelField.text);
            else
              level = -1;
            thisComponent.call("setfilterlevel", modelData, level);
          }

          onVisibleChanged: if(visible) updateLevelFilterUI();
          CheckBox {
            id: filterCatCB;
            text: modelData
            Layout.alignment: Qt.AlignLeft
            onCheckedChanged: {
              filterLevelField.enabled = checked;
            }
            onClicked: {
              if(checked)
              {
                filterLevelField.forceActiveFocus();
                filterLevelField.text = "0";
              }
              else
                filterLevelField.text = "";
            }
          }
          TextField {
            id: filterLevelField
            enabled: false
            Layout.preferredWidth: 30
            Layout.alignment: Qt.AlignRight
            horizontalAlignment: TextInput.AlignRight
            maximumLength: 2
            inputMethodHints: Qt.ImhDigitsOnly
            validator: IntValidator { bottom: 0 }

            onTextChanged: updateLevelFilters();
          }
        }
      }
    }
  }
}

