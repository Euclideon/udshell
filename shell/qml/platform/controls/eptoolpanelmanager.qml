import QtQuick 2.6

Item {
  property var panelList: []

  id: epToolPanelManager

  function addPanel(panel, width) {
    panelList.push({"panel" : panel, "visible" : false, "width" : width});
    panel.visible = false;
    panel.parent = this;
  }

  onVisibleChanged: {
    if(visible)
    {
      for(var i = 0; i < panelList.length; i++)
      {
        panelList[i].panel.visible = panelList[i].visible;
        panelList[i].panel.parent = this;
      }
    }
  }

  function toggleVisible(panel) {
    if(panel.visible) {
      for(var i = 0; i < panelList.length; i++)
      {
        if(panelList[i].panel == panel)
          panelList[i].visible = false;
      }

      panel.visible = false;
      visible = false;
      implicitWidth = 0;
    }
    else {
      for(var i = 0; i < panelList.length; i++)
      {
        if(panelList[i].panel != panel) {
          panelList[i].visible = false;
          panelList[i].panel.visible = false;
        }
        else {
          implicitWidth = panelList[i].width;
          panelList[i].visible = true;
        }
      }
      visible = true;
      panel.visible = true;
    }
  }
}
