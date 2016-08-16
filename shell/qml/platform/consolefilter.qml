import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2

ColumnLayout {
  id: filterWindow

  property var consolecomp

  spacing: 10
  anchors.top: parent.top
  anchors.left: parent.left
  anchors.margins: 12

  RowLayout {
    id: filterText
    Layout.alignment: Qt.AlignRight
    spacing: 6

    Text {
      id: ftLabel
      Layout.alignment: Qt.AlignRight
      text: "Filter Text:"
    }
    TextField {
      id: ftTextField
      Layout.preferredWidth: catLayout.implicitWidth - fcLabel.implicitWidth - filterText.spacing

      onTextChanged: consolecomp.filterText = ftTextField.text;

      Component.onCompleted: {
        filterWindow.consolecompChanged.connect(onConsoleChanged);
      }
      function onConsoleChanged() { ftTextField.text = consolecomp.filterText; }
    }
  }

  ColumnLayout {
    Component.onCompleted: {
      filterWindow.consolecompChanged.connect(onConsoleChanged);
    }
    function onConsoleChanged() {
      visible = consolecomp.outputLog;
    }

    RowLayout {
      id: filterComponents
      Layout.alignment: Qt.AlignRight
      spacing: 6

      Text {
        id: fcLabel
        Layout.alignment: Qt.AlignRight
        text: "Filter Log Components:"
      }
      TextField {
        id: fcTextField
        Layout.preferredWidth: catLayout.implicitWidth - fcLabel.implicitWidth - filterComponents.spacing

        onTextChanged: consolecomp.filterComponents = fcTextField.text;

        Component.onCompleted: {
          filterWindow.consolecompChanged.connect(onConsoleChanged);
        }
        function onConsoleChanged() { fcTextField.text = consolecomp.filterComponents; }
      }
    }

    RowLayout {
      id: catLayout
      spacing: 15
      GroupBox {
        title: "Show Log Categories"
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        GridLayout {
          rowSpacing: 4
          columnSpacing: 30
          columns: 2
          anchors.fill: parent
          anchors.margins: 3

          Repeater {
            model: ["Error", "Info", "Warning", "Script", "Debug", "Trace"]
            RowLayout {
              Layout.preferredWidth: 70
              Layout.preferredHeight: 27

              function updateCategoryFiltersUI()
              {
                filterCatCB.checked = consolecomp.isFilterCategoryEnabled(modelData);
              }

              function updateCategoryFilters()
              {
                if(filterCatCB.checked)
                  consolecomp.enableFilterCategory(modelData);
                else
                  consolecomp.disableFilterCategory(modelData);
              }

              Component.onCompleted: {
                filterWindow.consolecompChanged.connect(onConsoleChanged);
              }
              function onConsoleChanged() { updateCategoryFiltersUI(); }

              CheckBox {
                id: filterCatCB;
                text: modelData
                Layout.alignment: Qt.AlignLeft
                onCheckedChanged: {
                  updateCategoryFilters();
                }
              }
            }
          }
        }
      }

      GroupBox {
        title: "Filter Log Levels"
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        GridLayout {
          rowSpacing: 4
          columnSpacing: 30
          columns: 1
          anchors.fill: parent
          anchors.margins: 3

          Repeater {
            model: ["Warning", "Debug", "Info"]
            RowLayout {
              property alias filterLevelCB: filterLevelCB
              Layout.preferredWidth: 120
              Layout.preferredHeight: 27

              function updateLevelFiltersUI()
              {
                var level = consolecomp.getFilterLevel(modelData);
                if(level != -1)
                {
                  filterLevelCB.checked = true;
                  filterLevelField.text = level;
                }
                else
                  filterLevelCB.checked = false;
              }

              function updateLevelFilters()
              {
                var level;
                if(filterLevelCB.checked && filterLevelField.text.length != 0)
                  level = parseInt(filterLevelField.text);
                else
                  level = -1;
                consolecomp.setFilterLevel(modelData, level);
              }

              Component.onCompleted: {
                filterWindow.consolecompChanged.connect(onConsoleChanged);
              }
              function onConsoleChanged() { updateLevelFiltersUI(); }

              CheckBox {
                id: filterLevelCB;
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
  }
}

