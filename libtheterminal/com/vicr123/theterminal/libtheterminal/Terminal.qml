import QtQuick
import QtQuick.Controls
import Contemporary
import com.vicr123.Contemporary
import "impl" as Impl

Item {
    FocusScope {
        anchors.fill: parent

        Rectangle {
            id: screen
            color: "black"
            anchors.topMargin: SafeZone.top + 3
            anchors.fill: parent

            focus: true

            Impl.QmlTerminalScreenController {
                id: controller
                cols: screen.cols
                rows: screen.rows
            }

            property string shell: "/bin/bash";

            readonly property int rows: Math.floor(screen.height / (fontMetrics.height + 1))
            readonly property int cols: Math.floor(screen.width / (fontMetrics.averageCharacterWidth))

            Component.onCompleted: () => {
                                       controller.start(screen.shell);
                                   }


            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.IBeamCursor
                hoverEnabled: true
                z: 10

                onPressed: () => {
                    screen.forceActiveFocus(Qt.MouseFocusReason)
                }
            }

            Keys.onPressed: event => {
                controller.pressKey(event.modifiers, event.key, event.text);
                event.accepted = true;
            }

            ScrollView {
                anchors.fill: parent
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                FontMetrics {
                    id: fontMetrics
                    font.family: "JetBrains Mono"
                }

                Column {
                    id: screenRows
                    spacing: 0

                    Repeater {
                        model: controller.scrollbackLines
                        Row {
                            Impl.TerminalScreenRun {
                                text: "bash $ in scrollback"
                            }
                        }
                    }
                    Repeater {
                        model: screen.rows
                        Row {
                            id: screenRow

                            required property int index
                            Repeater {
                                id: screenRowRepeater
                                model: controller.runs(screenRow.index)

                                Impl.TerminalScreenRun {
                                    required property var modelData

                                    text: modelData.text
                                    backgroundColor: modelData.backgroundColor
                                    color: modelData.color
                                }
                            }

                            Connections {
                                target: controller
                                function onRowContentChanged(index) {
                                    if (screenRow.index !== index) return;
                                    screenRowRepeater.model = controller.runs(screenRow.index);
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    id: caret
                    visible: screen.activeFocus
                    x: fontMetrics.averageCharacterWidth * controller.caretCol
                    y: (fontMetrics.height + 1) * controller.caretRow
                    height: fontMetrics.height
                    width: fontMetrics.averageCharacterWidth
                    color: "white"

                    Behavior on x {
                        SmoothedAnimation {
                            velocity: 1000
                        }
                    }

                    Behavior on y {
                        SmoothedAnimation {
                            velocity: 1000
                        }
                    }
                }
            }
        }
    }
}
