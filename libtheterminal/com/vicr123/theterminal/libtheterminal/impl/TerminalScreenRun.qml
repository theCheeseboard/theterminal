import QtQuick 2.15

Item {
    id: root
    implicitWidth: textRun.width
    implicitHeight: textRun.height

    property string text
    property color backgroundColor: "#000000"
    property color color: "#FFFFFF"
    property bool blink: false

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
        visible: true

        SequentialAnimation {
            running: root.blink
            loops: Animation.Infinite

            PropertyAction {
                target: textRun
                property: "visible"
                value: true
            }
            PauseAnimation {
                duration: 500
            }
            PropertyAction {
                target: textRun
                property: "visible"
                value: false
            }
            PauseAnimation {
                duration: 500
            }
        }
    }
}
