import QtQuick 2.4
import QtQuick.Controls 1.3
import udKernel 0.1

Item {
  id: main
  anchors.fill: parent

  function mtest(wow) { console.log("nowai!! " + wow); return 10; }

  function test(c)
  {
    console.log(c.__Get("uid"));

    c.__Set("name", "bob");
    res = c.__Call("mtest", "Wheee!");

    console.log(res);
  }

  UDRenderView {
    id: viewport
    anchors.fill: parent
    anchors.margins: 10
  }
}
