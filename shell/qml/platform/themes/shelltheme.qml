pragma Singleton
import QtQuick 2.6

QtObject {
  // All controls
  property color controlBgColor: "transparent"
  property color controlBgColorPressed: "#333"
  property color controlBgColorHovered: "#888"

  // Activities
  property color activitySideBarBgColor: "transparent"
  property int activitySideBarWidth: 44

  // Bottom Bar
  property color bottomBarBgColor: toolBarBgColor
  property color bottomBarButtonTextColor: "white"
  property int bottomBarButtonFontSize: 11
  property color bottomBarButtonBorderColor: "#333"
  property color bottomBarButtonBgColorStart: "#555"  // Gradiant start color
  property color bottomBarButtonBgColorEnd: "#333"    // Gradiant end color
  property color bottomBarButtonBgColorHoveredStart: "#333"
  property color bottomBarButtonBgColorHoveredEnd: "#000"

  // DragTile
  property color dragTileBgColor: "white"
  property color dragTileTextColor: "black"
  property color dragTileBorderColor: "black"
  property int dragTileBorderWidth: 1
  property double dragTileHeightMultiplier: 1.5

  // ListView
  property color listViewItemTextColor: "white"
  property color listViewItemBgColor: "#444"
  property color listViewItemBgColorAlternate: "#383838"
  property color listViewItemBgColorSelected: "#777"
  property color listViewItemBgColorHovered: "blue"
  property int listViewHighlightMoveVelocity: 2000

  // MenuBar
  property color menuBarBgColor: "#444"
  property color menuBarButtonBgColor: "transparent"
  property color menuBarButtonBgColorSelected: "red"
  property color menuBarButtonTextColor: "white"
  property color menuBarButtonTextColorSelected: "white"
  property double menuBarButtonWidthMultiplier: 2.0
  property double menuBarButtonHeightMultiplier: 1.4
  // Menu
  property color menuBorderColor: "#282828"
  // Menu Item
  property color menuItemBgColor: "#1B1B1B"
  property color menuItemBgColorSelected: "#3C3C3C"
  property int menuItemBgRadius: 0
  property int menuItemBgRadiusSelected: 3
  property color menuItemTextColor: "white"
  property color menuItemDisabledColor: "grey"
  property color menuItemTextColorSelected: menuItemTextColor
  property color menuItemShortcutTextColor: menuItemTextColor
  property color menuItemShortcutTextColorSelected: menuItemTextColor
  property color menuItemSubmenuIndicatorColor: menuItemTextColor
  property color menuItemSubmenuIndicatorColorSelected: menuItemTextColor
  property color menuItemCheckboxCheckColor: menuItemTextColor
  property color menuItemCheckboxBorderColor: "#555"
  property color menuItemSeparatorColor: "#555"

  // MenuButton
  property color menuButtonDropdownIndicatorColor: "white"
  property int menuButtonDropdownIndicatorHeight: 9

  // Tab
  property color tabBgColor: "#333"
  // TabBar
  property color tabBarBgColor: "#555"
  property color tabBarBgColorSelected: tabBgColor
  property color tabBarTextColor: "black"
  property color tabBarTextColorSelected: "white"
  property int tabBarRadius: 2

  // TableView
  property color tableViewBgColor: tableViewItemBgColor
  property color tableViewItemBgColor: listViewItemBgColor
  property color tableViewItemBgColorAlternate: listViewItemBgColorAlternate
  property color tableViewItemBgColorSelected: listViewItemBgColorSelected
  property color tableViewItemBgColorHovered: listViewItemBgColorHovered
  property color tableViewItemTextColor: listViewItemTextColor
  // TableView Header
  property color tableViewHeaderBgColor: "#666"
  property color tableViewHeaderTextColor: "black"
  property color tableViewHeaderSortIndicatorColor: "black"
  property color tableViewHeaderSeparatorColor: "#999"
  property int tableViewHeaderSortIndicatorHeight: 9
  property double tableViewHeaderHeightMultiplier: 1.3

  // Toolbar
  property color toolBarBgColor: "transparent"
  property color toolBarSeparatorColor: "#555"

  // ToolButton
  property color toolButtonBgColor: controlBgColor
  property color toolButtonBgColorPressed: controlBgColorPressed
  property color toolButtonBgColorHovered: controlBgColorHovered
  property int toolButtonPadding: 3

  // Tool Panel
  property color toolPanelBgColor: "#444"
  property int toolPanelWidth: 200
  property color toolPanelToolBarBgColor: "#555"
  property int toolPanelToolBarSize: 40
  property color toolPanelTextFieldBgColor: "#333"
  property color toolPanelTextFieldBorderColor: "#888"
  property color toolPanelTextFieldTextColor: "white"
  property int toolPanelTextFieldRadius: 2

  // Window
  property color windowBgColor: "#444"
  property color windowSeparatorColor: "#555"
  property int windowToolBarSize: 36
  property color windowToolBarBgColor: toolBarBgColor
}
