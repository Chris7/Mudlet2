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
 }
