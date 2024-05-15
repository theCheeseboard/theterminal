import QtQuick 2.15
import com.vicr123.Contemporary
import QtQuick.Layouts
import QtQuick.Controls
import com.vicr123.theterminal.libtheterminal
import Contemporary

Item {
    id: root

    required property string profileUuid;
    readonly property var stackView: StackView.view

    TerminalProfile {
        id: profile
        zoom: 1
        profileUuid: root.profileUuid
    }

    Grandstand {
        id: grandstand
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        innerTopMargin: SafeZone.top

        text: profile.profileName
        color: Contemporary.calculateLayer(2).value
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
                    columns: 3

                    Label {
                        text: qsTr("Font")
                    }

                    ComboBox {
                        model: 5
                    }

                    SpinBox {
                        from: 1
                        to: 100
                        value: 12
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
