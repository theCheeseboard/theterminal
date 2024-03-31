import QtQuick 2.15

Item {
    id: root
    implicitWidth: textRun.width
    implicitHeight: textRun.height

    property string text
    property color backgroundColor: "#000000"
    property color color: "#FFFFFF"

    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    Text {
        id: textRun
        padding: 0
        text: root.text
        font.family: "JetBrains Mono"
        color: root.color
    }
}
