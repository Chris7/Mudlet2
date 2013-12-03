import QtQuick 2.0

Item {
	id: gaugeContainer
	objectName: "gaugeContainer"
    width: 50; height: 200
    property string gaugeText: "gaugeText"
    property string frontColor: "#ff0000"
    property string backColor: "#aa3232"
    property string backBorderColor: "#000000"
    property string frontBorderColor: "#000000"

    Rectangle {
        id: gaugeBack
        width: parent.width
        height: parent.height
        color: parent.backColor
        radius: 20
        border.width: 10
        border.color: parent.backBorderColor
        objectName: "gaugeBack"
        visible: true
        clip: false
        z: 1
        anchors.left: parent.left
        anchors.top: parent.top
    }

    Rectangle {
        id: gaugeFront
        objectName: "gaugeFront"
        color: parent.frontColor
        radius: 20
        border.color: parent.frontBorderColor
        opacity: 1
        z: 2
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width
        height: parent.height
    }

	Text {
		id: gaugeText
		objectName: "gaugeText"
        text: parent.gaugeText
        anchors.verticalCenter: parent.verticalCenter
        wrapMode: Text.WrapAnywhere
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        opacity: 1
        z: 3
		font.pointSize: 12
		font.bold: true
        width: 1
		anchors.horizontalCenter: parent.horizontalCenter
    }

    MouseArea {
        id: mouseArea
        x: 0
        y: 0
        width: parent.width
        height: parent.height
    }
}
