import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls
import com.vicr123.Contemporary
import Contemporary

Item {
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

        text: qsTr("General")
        color: layer2.color
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
                title: qsTr("Startup")

                GridLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    columns: 2
                    columnSpacing: 6

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Default Profile")
                    }

                    ComboBox {
                        model: 5
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
