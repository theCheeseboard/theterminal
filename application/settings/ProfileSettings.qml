import QtQuick 2.15
import com.vicr123.Contemporary

Item {
    Component {
        id: singleProfileComponent

        SingleProfileSettings {

        }
    }

    Pager {
        id: stack
        anchors.fill: parent
        currentAnimation: Pager.Lift

        RootProfileSettings {
            onOpenProfileSettings: profile => {
                stack.push(singleProfileComponent, {
                    "profileUuid": profile
                })
            }
        }
    }
}
