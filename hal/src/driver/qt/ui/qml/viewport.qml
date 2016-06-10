import QtQuick 2.6
import Platform 0.1

FocusScope {
  id: viewport
  anchors.fill: parent

  // Properties // ------------------------
  property var epTypeDesc: {
    "super": "ep.uicomponent",
    "id": "ui.viewport",
    "displayname": "UI.Viewport",
    "description": "QML Based Viewport"
  }
  property var view

  // Methods // ---------------------------
  function epInitComponent(initParams) {
    if ("view" in initParams)
      view = initParams.view;
    else {
      console.log("Creating internal view component");
      view = EPKernel.createComponent("ep.view", {});
    }
  }

  // Signal Handlers // ------------------
  onViewChanged: {
    renderView.attachView(view);
  }

  // Item Tree // ------------------------
  EPRenderView {
    id: renderView
    anchors.fill: parent

    onActiveFocusChanged: {
      if(activeFocus) {
        EPKernel.focus.push(this);
      }
    }
  }
}
