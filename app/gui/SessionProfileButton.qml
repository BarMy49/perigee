import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

import StreamingPreferences 1.0

Button {
    id: profileButton
    objectName: "sessionProfileButton"

    readonly property var profileKeys: ["", "programming", "gaming"]
    readonly property var profileNames: [qsTranslate("SettingsView", "Current settings"),
                                         qsTranslate("SettingsView", "Programming"),
                                         qsTranslate("SettingsView", "Gaming")]
    readonly property int profileIndex: Math.max(0, profileKeys.indexOf(StreamingPreferences.sessionProfile))

    activeFocusOnTab: true
    focusPolicy: Qt.StrongFocus
    hoverEnabled: true
    leftPadding: 18
    rightPadding: 22
    topPadding: 0
    bottomPadding: 0
    implicitHeight: 52

    text: profileNames[profileIndex]

    contentItem: Row {
        spacing: 10

        Image {
            anchors.verticalCenter: parent.verticalCenter
            source: "qrc:/res/session_profile.svg"
            sourceSize.width: 20
            sourceSize.height: 20
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter

            Label {
                text: qsTranslate("SettingsView", "Session profile")
                color: adwaita.accentForeground
                opacity: 0.85
                font.pointSize: 9
            }

            Label {
                text: profileButton.text
                color: adwaita.accentForeground
                font.pointSize: 11
                font.bold: true
            }
        }
    }

    Accessible.name: qsTranslate("SettingsView", "Session profile") + ": " + text
    ToolTip.delay: 1000
    ToolTip.timeout: 3000
    ToolTip.visible: hovered && !profileMenu.visible
    ToolTip.text: qsTranslate("SettingsView", "Session profile")

    AdwaitaPalette {
        id: adwaita
        dark: profileButton.Material.theme === Material.Dark
    }

    background: Rectangle {
        implicitHeight: 52
        radius: height / 2
        color: profileButton.down ? Qt.darker(adwaita.accent, 1.2) :
               profileButton.hovered ? Qt.lighter(adwaita.accent, 1.08) : adwaita.accent
        border.width: profileButton.visualFocus ? 3 : 0
        border.color: adwaita.dark ? "#99c1f1" : "#1a5fb4"

        Rectangle {
            z: -1
            anchors.fill: parent
            anchors.topMargin: 3
            anchors.bottomMargin: -3
            radius: parent.radius
            color: "#33000000"
        }
    }

    function selectProfile(index) {
        if (!StreamingPreferences.selectSessionProfile(profileKeys[index])) {
            ToolTip.show(qsTranslate("SettingsView", "Unable to save profile"), 3000)
        }
    }

    onClicked: {
        if (profileMenu.visible) {
            profileMenu.close()
        }
        else {
            profileMenu.open()
        }
    }

    Keys.onReturnPressed: clicked()
    Keys.onEnterPressed: clicked()

    NavigableMenu {
        id: profileMenu
        objectName: "sessionProfileMenu"
        initiator: profileButton
        y: -height - 8
        x: profileButton.width - width

        background: Rectangle {
            implicitWidth: 200
            radius: 12
            color: adwaita.dark ? "#36363a" : "#ffffff"
            border.width: 1
            border.color: adwaita.cardBorder
        }

        NavigableMenuItem {
            text: profileButton.profileNames[0]
            checkable: true
            checked: profileButton.profileIndex === 0
            onTriggered: {
                profileButton.selectProfile(0)
                checked = Qt.binding(function() { return profileButton.profileIndex === 0 })
            }
        }
        NavigableMenuItem {
            text: profileButton.profileNames[1]
            checkable: true
            checked: profileButton.profileIndex === 1
            onTriggered: {
                profileButton.selectProfile(1)
                checked = Qt.binding(function() { return profileButton.profileIndex === 1 })
            }
        }
        NavigableMenuItem {
            text: profileButton.profileNames[2]
            checkable: true
            checked: profileButton.profileIndex === 2
            onTriggered: {
                profileButton.selectProfile(2)
                checked = Qt.binding(function() { return profileButton.profileIndex === 2 })
            }
        }
    }
}
