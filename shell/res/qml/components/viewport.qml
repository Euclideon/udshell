import QtQuick 2.4
import QtQuick.Controls 1.3
import epKernel 0.1

FocusScope {
  id: viewport
  anchors.fill: parent

  signal resourcedropped(var uid, var x, var y)

  DropArea {
    anchors.fill: parent
    keys: ["text/uri-list", "RMResource"]

    onDropped: {
      if (drop.keys.indexOf("text/uri-list") > -1) {
        if (drop.hasText) {
          if (drop.proposedAction == Qt.MoveAction || drop.proposedAction == Qt.CopyAction) {
            var urls = drop.text.trim().split("\n");

            var resourceManager = EPKernel.findComponent("resourcemanager");

            for(var i = 0; i < urls.length; i++) {
              var dataSource = resourceManager.call("loadresourcesfromfile", {"src" : urls[i], "useStreamer" : true });
              var numResources = dataSource.get("numresources");
              var resource = dataSource.call("getresourcebyvariant", 0);
              resourcedropped(resource.get("uid"), drop.x, drop.y);
            }

            drop.accept();
          }
        }
      }
      else if(drop.keys.indexOf("RMResource") > -1)
        resourcedropped(drop.source.payload.uid, drop.x, drop.y);
    }
  }

  EPRenderView {
    id: renderview
    anchors.fill: parent
  }
}
