import QtQuick
import QtQuick.Controls
import Contemporary
import com.vicr123.Contemporary
import "impl" as Impl

Item {
    Rectangle {
        id: screen
        color: "black"
        anchors.topMargin: SafeZone.top + 3
        anchors.fill: parent

        readonly property int charHeight: Math.floor(screen.height / (fontMetrics.height + 1))
        readonly property int charWidth: Math.floor(screen.width / (fontMetrics.averageCharacterWidth))

        ScrollView {
            anchors.fill: parent
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            FontMetrics {
                id: fontMetrics
                font.family: "JetBrains Mono"
            }

            Column {
                spacing: 0

                Repeater {
                    model: 90
                    Row {
                        Impl.TerminalScreenRun {
                            text: "bash $ in scrollback"
                        }
                    }
                }
                Repeater {
                    model: screen.charHeight
                    Row {
                        Impl.TerminalScreenRun {
                            text: "s".repeat(screen.charWidth / 2)
                        }
                        Impl.TerminalScreenRun {
                            text: "s".repeat(screen.charWidth / 2)
                            backgroundColor: "#FF0000"
                            color: "white"
                        }
                    }
                }
            }
        }
    }
}
