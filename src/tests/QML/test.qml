 import QtQuick 2.0

 Rectangle {
     id: page
     width: 500; height: 200
     color: "lightgray"

     Text {
         id: helloText
		 objectName: "helloText"
         text: "Hello wod!"
         y: 30
         anchors.horizontalCenter: page.horizontalCenter
         font.pointSize: 24; font.bold: true
     }

     MouseArea {
		id: mouseArea
		anchors.fill: parent
		onClicked: Lua.callLua("echo([["+helloText.text+"]])")
	 }
	 
	Gauge {
		x: 30
		y: 10
	}
 }