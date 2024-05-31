import QtQuick 2.15
import com.vicr123.Contemporary
import ".."

Item {
    Component {
        id: singleProfileComponent

        SingleProfileSettings {

        }
    }

    ProfileHelper {
        id: profileHelper
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

            onCreateNewProfile: () => {
                stack.push(singleProfileComponent, {
                    "profileUuid": profileHelper.newProfileUuid()
                })
            }
        }
    }
}
