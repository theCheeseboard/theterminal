import QtQuick 2.15

Item {
    id: root
    implicitWidth: textRun.width
    implicitHeight: textRun.height

    property string text
    property color backgroundColor: "#000000"
    property color color: "#FFFFFF"
    property bool blink: false
    property bool underline: false
    property bool bold: false
    property font font;

    font.weight: root.bold ? 900 : 400

    property int selStart: -1
    property int selEnd: -1

    FontMetrics {
        id: fontMetrics
        font: root.font
    }

    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    Item {
        id: textContainer
        implicitHeight: textRun.height
        implicitWidth: textRun.width
        visible: true

        Text {
            id: textRun
            padding: 0
            text: root.text
            font: root.font
            color: root.color
            textFormat: Text.PlainText
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            implicitHeight: 1
            color: root.color
            visible: root.underline
        }

        SequentialAnimation {
            running: root.blink
            loops: Animation.Infinite

            PropertyAction {
                target: textContainer
                property: "visible"
                value: true
            }
            PauseAnimation {
                duration: 500
            }
            PropertyAction {
                target: textContainer
                property: "visible"
                value: false
            }
            PauseAnimation {
                duration: 500
            }
        }
    }
}
