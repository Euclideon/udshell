import QtQuick 2.6
import QtQuick.Window 2.2

Window {
  id: splash
  title: "udShell Splash"
  modality: Qt.ApplicationModal
  flags: Qt.SplashScreen
  //x: (Screen.width - splashImage.width) / 2
  //y: (Screen.height - splashImage.height) / 2
  //width: splashImage.width
  //height: splashImage.height

  //Rectangle {
    //id: splashImage
    //width: 1
    //height: 1
    //color: "white"

    //Text {
      //anchors.horizontalCenter: parent.horizontalCenter
      //anchors.verticalCenter: parent.verticalCenter
      //font.pixelSize: 58
      //color: "red"
      //text: "Loading udShell!!!"
    //}
  //}

  Component.onCompleted: visible = true
}
