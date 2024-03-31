import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import Contemporary
import Qt.labs.platform as Labs
import com.vicr123.theterminal.libtheterminal

import com.vicr123.Contemporary.CoreStyles

ContemporaryWindow {
    id: window

    width: 800
    height: 600
    title: qsTr("theTerminal")
    visible: true

    Labs.MenuBar {
        Labs.Menu {
            title: qsTr("&File")

            Labs.MenuItem {
                text: qsTr("&Quit")

                onTriggered: Qt.quit()
            }
        }
        Labs.Menu {
            title: qsTr("&Theme")

            Labs.MenuItem {
                id: lightAction
                text: qsTr("Light")
                checked: Contemporary.colorTheme == Contemporary.Light
                checkable: true
                onCheckedChanged: lightAction.checked && Contemporary.setColorTheme(Contemporary.Light)
            }
            Labs.MenuItem {
                id: darkAction
                text: qsTr("Dark")
                checked: Contemporary.colorTheme == Contemporary.Dark
                checkable: true
                onCheckedChanged: darkAction.checked && Contemporary.setColorTheme(Contemporary.Dark)
            }
        }
        Labs.Menu {
            title: qsTr("&Help")

            Labs.MenuItem {
                text: qsTr("&About")
                onTriggered: outerStack.push(aboutSurface)
            }
        }
    }

    ContemporaryStackView {
        id: outerStack
        anchors.fill: parent

        currentAnimation: ContemporaryStackView.Animation.Lift

        initialItem: ContemporaryWindowSurface {
            actionBar: ActionBar {
                menuItems: [
                    Menu {
                        title: qsTr("Help")

                        Action {
                            text: qsTr("About")
                            onTriggered: outerStack.push(aboutSurface)
                        }
                    },
                    Action {
                        shortcut: "Ctrl+Q"
                        text: qsTr("Exit")

                        onTriggered: window.close()
                    }
                ]

                ActionBarTabber {
                    ActionBarTabber.Button {
                        text: qsTr("Components")
                        checked: stack.currentIndex === 0
                        onActivated: stack.currentIndex = 0
                    }
                    ActionBarTabber.Button {
                        text: qsTr("Patterns")
                        checked: stack.currentIndex === 1
                        onActivated: stack.currentIndex = 1
                    }
                }
            }
            overlayActionBar: true

            Pager {
                id: stack
                anchors.fill: parent

                Terminal {}
            }
        }

        AboutSurface {
            id: aboutSurface
        }
    }
}
