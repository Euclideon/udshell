import QtQuick 2.6
import QtQuick.Dialogs 1.2

FileDialog {
  function urlToNativePath(url) {
    var path = url.toString();
    // remove prefixed "file:///" or "file://"
    if(Qt.platform.os === "windows")
      path = path.replace(/^(file:\/{3})/,"");
    else
      path = path.replace(/^(file:\/{2})/,"");
    // unescape html codes like '%23' for '#'
    path = decodeURIComponent(path);

    return path;
  }
}
