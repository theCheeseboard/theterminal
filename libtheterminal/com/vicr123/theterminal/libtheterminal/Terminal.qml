import QtQuick
import QtQuick.Controls
import Contemporary
import com.vicr123.Contemporary
import "impl" as Impl

Item {
    function paste() {
        controller.paste()
    }

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
                rowList.positionViewAtEnd();
                event.accepted = true;
            }

            ListView {
                id: rowList
                anchors.fill: parent
                clip: true

                FontMetrics {
                    id: fontMetrics
                    font.family: "JetBrains Mono"
                }

                ScrollBar.vertical: ScrollBar {
                    id: rowListScrollBar
                    active: true
                }

                model: screen.rows + controller.scrollbackLines

                onModelChanged: () => {
                                    rowList.positionViewAtEnd();
                                }

                delegate: Row {
                    id: screenRow

                    required property int index
                    property var rowScaleMode: controller.rowScaleMode(index)

                    clip: true
                    transform: Scale {
                        xScale: screenRow.rowScaleMode !== 0 ? 2 : 1
                        yScale: screenRow.rowScaleMode >= 2 ? 2 : 1
                    }

                    Repeater {
                        id: screenRowRepeater
                        model: screenRow.index >= controller.scrollbackLines ? controller.runs(screenRow.index - controller.scrollbackLines) : controller.scrollbackRuns(screenRow.index)

                        Impl.TerminalScreenRun {
                            required property var modelData

                            text: modelData.text
                            backgroundColor: modelData.backgroundColor
                            color: modelData.color
                            blink: modelData.blink
                            underline: modelData.underline
                            bold: modelData.bold

                            transform: Translate {
                                y: screenRow.rowScaleMode === 3 ? -screenRow.height / 2 : 0
                            }
                        }
                    }

                    Connections {
                        target: controller
                        function onRowContentChanged(index) {
                            const translatedIndex = index + controller.scrollbackLines;
                            if (screenRow.index !== translatedIndex) return;
                            screenRow.rowScaleMode = controller.rowScaleMode(index)
                            screenRowRepeater.model = controller.runs(index);
                        }
                    }
                }

                Rectangle {
                    id: caret
                    visible: screen.activeFocus && controller.caretVisible
                    x: fontMetrics.averageCharacterWidth * controller.caretCol
                    y: rowList.contentY - rowList.contentY + rowList.itemAtIndex(controller.caretRow + controller.scrollbackLines)?.mapToItem(screen, 0, 0).y ?? 0
                    height: rowList.itemAtIndex(controller.caretRow + controller.scrollbackLines)?.childrenRect.height ?? 0
                    width: fontMetrics.averageCharacterWidth
                    color: "white"

                    Behavior on x {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.OutCubic
                        }
                    }

                    Behavior on y {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.OutCubic
                        }
                    }
                }
            }
        }
    }
}
