 import QtQuick 2.0

 Rectangle {
     id: page
     width: 500; height: 200
     color: "lightgray"
	 
    HGauge {
        id: hgauge1
        x: 0
        y: 150
      width: 500
      height: 50
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      gaugeText: "HP"
	}

 VGauge {
     id: vgauge1
     x: 0
     y: 0
     width: 50
     height: 144
     anchors.bottomMargin: 6
     anchors.verticalCenter: parent.verticalCenter
     anchors.left: parent.left
     anchors.bottom: hgauge1.top
     anchors.top: parent.top
     gaugeText: "SP"
 }
 }
