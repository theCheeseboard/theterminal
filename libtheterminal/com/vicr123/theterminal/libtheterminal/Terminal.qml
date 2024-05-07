import QtQuick
import QtQuick.Controls
import Contemporary
import com.vicr123.Contemporary
import QtQuick.Dialogs
import "impl" as Impl

Item {
    id: root

    property font font: {
        family: "JetBrains Mono"
    }
    property string shell: "/bin/bash";
    readonly property bool haveSelection: controller.haveSelection
    property string colorName: "Linux";

    function paste() {
        controller.paste()
    }
    function copy() {
        controller.copy()
    }
    function tryClose() {
        const processes = controller.runningProcesses();
        if (processes.length > 0) {
            quitWithRunningProcessesDialog.informativeText = qsTr("Closing this terminal will also close %n processes: %1", "", processes.length).arg(processes.join(", "));
            quitWithRunningProcessesDialog.visible = true
            return;
        }

        root.close();
    }

    signal close()

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
                colorName: root.colorName
            }

            readonly property int rows: Math.floor(screen.height / (fontMetrics.height + 1))
            readonly property int cols: Math.floor(screen.width / (fontMetrics.averageCharacterWidth))

            Component.onCompleted: () => {
                                       controller.start(root.shell);
                                   }

            function cellAt(x, y) {
                const item = rowList.itemAt(x, y + rowList.contentY);
                if (!item) return null;
                const xCell = Math.floor(x / item.width * screen.cols);
                return Qt.point(xCell, item.index);
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: controller.reportMouseEvents ? Qt.ArrowCursor : Qt.IBeamCursor
                hoverEnabled: true
                z: 10

                onPressed: event => {
                    screen.forceActiveFocus(Qt.MouseFocusReason)

                    if (event.buttons & Qt.LeftButton) {
                        controller.selectionStart = screen.cellAt(event.x, event.y) ?? controller.selectionStart;
                        controller.selectionEnd = screen.cellAt(event.x, event.y) ?? controller.selectionEnd;
                    }
                }
                onPositionChanged: event => {
                    if (event.buttons & Qt.LeftButton) {
                        controller.selectionEnd = screen.cellAt(event.x, event.y) ?? controller.selectionEnd;
                    }
                }
                onReleased: event => {

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
                    font: root.font
                }

                ScrollBar.vertical: ScrollBar {
                    id: rowListScrollBar
                    active: true
                }

                model: screen.rows + controller.scrollbackLines

                onModelChanged: () => {
                                    rowList.positionViewAtEnd();
                                }

                delegate: Item {
                    id: screenItem
                    required property int index
                    readonly property int selStart: controller.normalisedSelectionStart.y === screenItem.index ? controller.normalisedSelectionStart.x : -1
                    readonly property int selEnd: controller.normalisedSelectionEnd.y === screenItem.index ? controller.normalisedSelectionEnd.x : (controller.normalisedSelectionStart.y <= screenItem.index && screenItem.index < controller.normalisedSelectionEnd.y ? -2 : -1)

                    implicitWidth: root.width
                    implicitHeight: screenRow.height

                    Row {
                        id: screenRow

                        property var rowScaleMode: controller.rowScaleMode(screenItem.index)

                        clip: true
                        transform: Scale {
                            xScale: screenRow.rowScaleMode !== 0 ? 2 : 1
                            yScale: screenRow.rowScaleMode >= 2 ? 2 : 1
                        }

                        Repeater {
                            id: screenRowRepeater
                            model: screenItem.index >= controller.scrollbackLines ? controller.runs(screenItem.index - controller.scrollbackLines) : controller.scrollbackRuns(screenItem.index)

                            Impl.TerminalScreenRun {
                                required property var modelData

                                text: modelData.text
                                font: root.font
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
                                if (screenItem.index !== translatedIndex) return;
                                screenRow.rowScaleMode = controller.rowScaleMode(index)
                                screenRowRepeater.model = controller.runs(index);
                            }
                        }
                    }

                    Rectangle {
                        id: selection
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        visible: controller.normalisedSelectionStart !== controller.normalisedSelectionEnd
                        x: fontMetrics.averageCharacterWidth * screenItem.selStart
                        width: screenItem.selEnd == -2 ? root.width : fontMetrics.averageCharacterWidth * (screenItem.selEnd - screenItem.selStart + 1)
                        color: "#70FFFFFF";
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

    MessageDialog {
        id: quitWithRunningProcessesDialog
        text: "Close terminal with running processes?"
        buttons: MessageDialog.Ok | MessageDialog.Cancel
        onButtonClicked: (button, role) => {
            switch (button) {
                case MessageDialog.Ok:
                    root.close();
                    break;
            }
        }
    }
}
