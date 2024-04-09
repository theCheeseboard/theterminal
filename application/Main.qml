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

    function hk_(shortcut) {
        if (Qt.platform.os === "osx") {
            return shortcut[0];
        } else {
            return `Shift+${shortcut[0]}`;
        }
    }

    NativeMenuBar {
        Labs.Menu {
            title: qsTr("File")

            Labs.MenuItem {
                text: qsTr("New Tab")
                shortcut: hk_`Ctrl+T`
                onTriggered: () => {
                    surface.newTab()
                    stack.currentIndex = stack.pages.length - 1
                }
            }
            Labs.MenuItem {
                text: qsTr("Close Tab")
                shortcut: hk_`Ctrl+W`
                onTriggered: () => {
                    surface.closeTab(stack.currentIndex)
                }
            }

            Labs.MenuItem {
                shortcut: hk_`Ctrl+Q`
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }
        Labs.Menu {
            title: qsTr("Edit");

            Labs.MenuItem {
                text: qsTr("Copy")
                shortcut: hk_`Ctrl+C`
                onTriggered: stack.pages[stack.currentIndex].copy()
            }
            Labs.MenuItem {
                text: qsTr("Paste")
                shortcut: hk_`Ctrl+V`
                onTriggered: stack.pages[stack.currentIndex].paste()
            }
        }
        Labs.Menu {
            title: qsTr("Help")

            Labs.MenuItem {
                text: qsTr("About")
                onTriggered: outerStack.push(aboutSurface)
            }
        }
    }

    ContemporaryStackView {
        id: outerStack
        anchors.fill: parent

        currentAnimation: ContemporaryStackView.Animation.Lift

        initialItem: ContemporaryWindowSurface {
            id: surface
            ListModel {
                id: terminals
            }

            function newTab() {
                stack.pages.push(terminalComponent.createObject(stack))
                terminals.append({
                    title: qsTr("Terminal")
                });
            }

            function closeTab(index) {
                const page = stack.pages[index];
                page.tryClose();
            }

            actionBar: ActionBar {
                menuItems: [
                    Action {
                        shortcut: hk_`Ctrl+T`
                        text: qsTr("New Tab")
                        icon.name: "tab-new"

                        onTriggered: () => {
                                         surface.newTab()
                                         stack.currentIndex = stack.pages.length - 1
                                     }
                    },
                    MenuSeparator {},
                    Action {
                        shortcut: hk_`Ctrl+C`
                        text: qsTr("Copy")
                        icon.name: "edit-copy"
                        onTriggered: stack.pages[stack.currentIndex].copy()
                    },
                    Action {
                        shortcut: hk_`Ctrl+V`
                        text: qsTr("Paste")
                        icon.name: "edit-paste"
                        onTriggered: stack.pages[stack.currentIndex].paste()
                    },
                    MenuSeparator {},
                    Action {
                        shortcut: hk_`Ctrl+W`
                        text: qsTr("Close Tab")
                        icon.name: "tab-close"
                        onTriggered: surface.closeTab(stack.currentIndex)
                    },
                    Menu {
                        title: qsTr("Help")
                        icon.name: "help-about"

                        Action {
                            text: qsTr("About theTerminal")
                            onTriggered: outerStack.push(aboutSurface)
                        }
                    },
                    Action {
                        shortcut: hk_`Ctrl+Q`
                        text: qsTr("Exit")
                        icon.name: "application-exit"

                        onTriggered: Qt.quit()
                    }
                ]

                ActionBarTabber {
                    Repeater {
                        model: terminals

                        ActionBarTabber.Button {
                            required property int index
                            required property string title
                            text: title
                            checked: stack.currentIndex === index
                            onActivated: stack.currentIndex = index
                        }
                    }
                }

                Button {
                    id: newTabButton
                    flat: true
                    icon.name: "tab-new"
                    implicitWidth: newTabButton.height
                    onClicked: () => {
                                   surface.newTab()
                                   stack.currentIndex = stack.pages.length - 1
                               }
                }
            }
            overlayActionBar: true

            Pager {
                id: stack
                anchors.fill: parent
                pages: []
            }

            Component {
                id: terminalComponent
                Terminal {
                    id: terminal

                    onClose: () => {
                        const index = stack.pages.indexOf(terminal);
                        stack.pages.splice(index, 1);
                        terminals.remove(index);
                        terminal.destroy();
                    }
                }
            }

            Component.onCompleted: () => {
                newTab();
            }
        }

        AboutSurface {
            id: aboutSurface
        }
    }
}
