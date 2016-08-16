import QtQuick 2.6
import Platform 0.1

FocusScope
{
  id: viewport
  anchors.fill: parent

  // Properties // ------------------------
  property var epTypeDesc: {
    "id": "ui.Viewport",
    "super": "ep.UIComponent",
    "description": "QML Based Viewport"
  }
  property alias view: renderView.view

  // Methods // ---------------------------
  function epInitComponent(initParams)
  {
    if ("view" in initParams)
      renderView.view = initParams.view;
    else
    {
      console.log("Creating internal view component");
      renderView.view = EPKernel.createComponent("ep.View", {});
    }
  }

  // Item Tree // ------------------------
  EPRenderView
  {
    id: renderView
    anchors.fill: parent

    onActiveFocusChanged: {
      if (activeFocus)
        EPKernel.focus.push(this);
    }
  }
}
