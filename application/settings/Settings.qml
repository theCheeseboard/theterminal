import QtQuick 2.15
import com.vicr123.Contemporary
import QtQuick.Controls

ContemporaryWindowSurface {
    id: root

    actionBar: ActionBar {
        Button {
            icon.name: "go-previous"
            text: qsTr("Settings")

            onClicked: root.stackView.pop()
            flat: true
        }

        ActionBarTabber {
            ActionBarTabber.Button {
                text: qsTr("General")
                checked: stack.currentIndex === 0
                onActivated: stack.currentIndex = 0
            }
            ActionBarTabber.Button {
                text: qsTr("Profiles")
                checked: stack.currentIndex === 1
                onActivated: stack.currentIndex = 1
            }
        }
    }

    overlayActionBar: true

    readonly property var stackView: StackView.view

    Pager {
        id: stack
        anchors.fill: parent

        GeneralSettings { }
        ProfileSettings { }
    }
}
