import QtQuick 2.0

Rectangle  {
    id: buttonContainer
    property string text: "Button"
    property string onClick: ""
    property string textColor: "black"

    width: buttonLabel.width + 20; height: buttonLabel.height + 5
    border  { width: 1; color: Qt.darker(activePalette.button) }
    color: "lightgray"
    smooth: true
    radius: 8

    MouseArea  {
        id: mouseArea
        anchors.fill: parent
        onClicked: Lua.callLua(parent.onClick)
    }

    Text  {
        id: buttonLabel
        anchors.centerIn: parent
        color: parent.textColor
        text: parent.text
    }
}
