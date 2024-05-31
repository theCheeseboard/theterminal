import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls
import com.vicr123.theterminal.libtheterminal
import com.vicr123.Contemporary
import Contemporary
import ".."

Item {
    id: root

    signal openProfileSettings(string profile);
    signal createNewProfile

    LayerCalculator {
        id: layer1
        layer: 1
    }

    LayerCalculator {
        id: layer2
        layer: 2
    }

    Grandstand {
        id: grandstand
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        innerTopMargin: SafeZone.top

        text: qsTr("Profiles")
        color: layer2.color
        z: 10
    }

    ListView {
        id: profileList
        anchors.top: grandstand.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 10

        spacing: 10

        model: ProfilesModel {

        }

        delegate: Item {
            id: item
            required property var uuid;

            implicitWidth: profileList.width
            implicitHeight: childrenRect.height

            Layer {
                id: layer
                anchors.centerIn: parent

                implicitWidth: 600
                implicitHeight: childrenRect.height + 9 + 9

                color: layer1.color

                TerminalProfile {
                    id: profile
                    zoom: 1
                    profileUuid: item.uuid
                }

                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.topMargin: 9
                    anchors.leftMargin: 9
                    spacing: 10

                    Label {
                        text: profile.profileName
                        font.pointSize: 20
                    }

                    RowLayout {
                        Item {
                            Layout.fillWidth: true
                        }

                        Button {
                            text: qsTr("Edit Profile")
                            onClicked: () => root.openProfileSettings(item.uuid)
                        }
                    }
                }
            }
        }

        footer: Item {
            implicitHeight: childrenRect.height + 6
            implicitWidth: profileList.width
            z: 20

            Button {
                id: addProfileButton
                anchors.centerIn: parent
                anchors.bottom: parent.top
                implicitWidth: 600
                text: qsTr("New Profile")
                icon.name: "list-add"

                onClicked: () => root.createNewProfile()
            }
        }
    }
}
