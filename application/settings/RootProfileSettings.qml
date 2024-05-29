import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls
import com.vicr123.Contemporary
import Contemporary

Item {
    id: root

    signal openProfileSettings(string profile);

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

        model: ListModel {
            ListElement {
                name: "Default"
                uuid: "EB31ADE1-9342-43E9-9E9C-811CCB978F64"
            }
        }

        delegate: Item {
            id: item
            required property var name;
            required property var uuid;

            implicitWidth: profileList.width
            implicitHeight: childrenRect.height

            Layer {
                id: layer
                anchors.centerIn: parent

                implicitWidth: 600
                implicitHeight: childrenRect.height + 9 + 9

                color: layer1.color

                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.topMargin: 9
                    anchors.leftMargin: 9
                    spacing: 10

                    Label {
                        text: item.name
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
    }
}
