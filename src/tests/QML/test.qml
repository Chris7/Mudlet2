 import QtQuick 2.0

 Item {
     id: container
     width: 500; height: 200
	 
    HGauge {
        id: hgauge1
        x: 0
        y: 150
      width: 500
      height: 50
      gaugeText: "HP"
      onClick: "if (hp < hpmax*50/100) then echo([[you might want to drink something]]) end"
	}

 VGauge {
     id: vgauge1
     x: 0
     y: 0
     width: 50
     height: 144
     gaugeText: "SP"
 }

 Button {
     id: button1
     state: "base"
     x: 68
     y: 0
     width: 143
     height: 40
     color: "#2e9797"
     text: "Click me!"
     border.color: "#ff0000"
     textColor: "purple"
     onClick: "echo([[clicked]])"
 }

 Button {
     id: button2
     state: "base"
     x: 68
     y: button2.height
     width: 143
     height: 40
     color: "#2e9797"
     text: "I flip!"
     border.color: "#ff0000"
     textColor: "purple"
     MouseArea  {
         id: mouseArea
         anchors.fill: parent
         onClicked: {
             if (button2.state === "base")
                 button2.state = "state2"
             else
                 button2.state = "base"
         }
     }
     states: [
         State {
             name: "base"

             PropertyChanges {
                 target: button2
                 rotation: 0
             }
         },
         State {
             name: "state2"

             PropertyChanges {
                 target: button2
                 rotation: -180
             }
         }

     ]
 }
 }
