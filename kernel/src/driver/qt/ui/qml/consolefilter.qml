import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1

ColumnLayout {
  id: filterWindow

  property var consolecomp
  signal consoleChanged()

  spacing: 10
  anchors.top: parent.top
  anchors.left: parent.left
  anchors.margins: 12

  onConsolecompChanged: {
    consoleChanged();
  }

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

      onTextChanged: consolecomp.set("filtertext", ftTextField.text);

      Component.onCompleted: {
        filterWindow.consoleChanged.connect(onConsoleChanged);
      }
      function onConsoleChanged() { ftTextField.text = consolecomp.get("filtertext"); }
    }
  }

  ColumnLayout {
    Component.onCompleted: {
      filterWindow.consoleChanged.connect(onConsoleChanged);
    }
    function onConsoleChanged() {
      visible = consolecomp.get("outputlog");
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

        onTextChanged: consolecomp.set("filtercomponents", fcTextField.text);

        Component.onCompleted: {
          filterWindow.consoleChanged.connect(onConsoleChanged);
        }
        function onConsoleChanged() { fcTextField.text = consolecomp.get("filtercomponents"); }
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
                filterCatCB.checked = consolecomp.call("isfiltercategoryenabled", modelData);
              }

              function updateCategoryFilters()
              {
                if(filterCatCB.checked)
                  consolecomp.call("enablefiltercategory", modelData);
                else
                  consolecomp.call("disablefiltercategory", modelData);
              }

              Component.onCompleted: {
                filterWindow.consoleChanged.connect(onConsoleChanged);
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
                var level = consolecomp.call("getfilterlevel", modelData);
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
                consolecomp.call("setfilterlevel", modelData, level);
              }

              Component.onCompleted: {
                filterWindow.consoleChanged.connect(onConsoleChanged);
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

