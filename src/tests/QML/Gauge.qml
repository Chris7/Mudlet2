import QtQuick 2.0

Item {
	id: gaugeContainer
	objectName: "gaugeContainer"
	width: 500; height: 200

	Rectangle {
		id: gaugeBack
		objectName: "gaugeBack"
		width: parent.width
		height: parent.height
		color: "steelblue"
		anchors.left: parent.left
		anchors.top: parent.top
	}
	
	Rectangle {
		id: gaugeFront
		objectName: "gaugeFront"
		color: "red"
		anchors.left: parent.left
		anchors.top: parent.top
		width: parent.width
		height: parent.height
	}

	Text {
		id: gaugeText
		objectName: "gaugeText"
		text: "Gauge Text"
		font.pointSize: 12
		font.bold: true
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.verticalCenter: parent.verticalCenter
	}
}