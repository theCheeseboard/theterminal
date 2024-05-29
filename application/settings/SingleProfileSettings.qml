import QtQuick 2.15
import com.vicr123.Contemporary
import QtQuick.Layouts
import QtQuick.Controls
import com.vicr123.theterminal.libtheterminal
import ".."
import Contemporary

Item {
    id: root

    required property string profileUuid;
    readonly property var stackView: StackView.view

    LayerCalculator {
        id: layer2
        layer: 2
    }

    TerminalProfile {
        id: profile
        zoom: 1
        profileUuid: root.profileUuid
    }

    FontModel {
        id: fontModel
    }

    ColorModel {
        id: colorModel
    }

    Grandstand {
        id: grandstand
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        innerTopMargin: SafeZone.top

        text: profile.profileName
        color: layer2.color
        backButtonVisible: true

        onBackButtonClicked: root.stackView.pop()
    }

    Flickable {
        id: flickable
        anchors.top: grandstand.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        ColumnLayout {
            anchors.top: parent.top
            anchors.left: parent.left
            implicitWidth: flickable.width
            spacing: 10

            Item {
                Layout.preferredHeight: 10
            }

            GroupBox {
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: 600
                title: qsTr("Shell")

                ColumnLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    Label {
                        text: qsTr("Set the shell that is used when you start a terminal with this profile")
                    }

                    TextField {
                        id: shellField
                        Layout.fillWidth: true
                        placeholderText: profile.defaultShell
                        text: profile.shell
                        onEditingFinished: () => {
                            profile.shell = shellField.text;
                            profile.saveProfile();
                        }
                    }
                }
            }

            GroupBox {
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: 600
                title: qsTr("Text")

                GridLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    columns: 3

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Font")
                    }

                    ComboBox {
                        id: fontBox

                        model: fontModel
                        textRole: "family"
                        valueRole: "family"

                        implicitWidth: 300

                        onActivated: () => {
                            profile.fontFamily = fontBox.currentValue;
                            profile.saveProfile();
                        }

                        Component.onCompleted: () => {
                            fontBox.currentIndex = fontBox.indexOfValue(profile.font.family)
                        }
                    }

                    SpinBox {
                        id: fontSizeBox
                        from: 1
                        to: 100
                        value: profile.font.pointSize

                        onValueChanged: () => {
                            profile.fontPointSize = fontSizeBox.value;
                            profile.saveProfile();
                        }
                    }
                }
            }

            GroupBox {
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: 600
                title: qsTr("Colours")

                GridLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    columns: 2

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Theme")
                    }

                    ComboBox {
                        id: colorsBox

                        model: colorModel
                        textRole: "description"
                        valueRole: "identifier"

                        implicitWidth: 250

                        onActivated: () => {
                            profile.colorName = colorsBox.currentValue;
                            profile.saveProfile();
                        }

                        Component.onCompleted: () => {
                            colorsBox.currentIndex = colorsBox.indexOfValue(profile.colorName)
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
