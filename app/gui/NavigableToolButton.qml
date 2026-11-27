import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3

ToolButton {
    id: toolButton
    property string iconSource

    activeFocusOnTab: true

    icon.source: iconSource
    icon.width: 20
    icon.height: 20

    Layout.alignment: Qt.AlignVCenter

    AdwaitaPalette {
        id: adwaita
        dark: toolButton.Material.theme === Material.Dark
    }

    background: Rectangle {
        implicitWidth: 36
        implicitHeight: 36
        radius: 6
        color: toolButton.down ? adwaita.buttonPressed : toolButton.hovered || toolButton.visualFocus ? adwaita.buttonHover : "transparent"
        border.width: toolButton.visualFocus ? 2 : 0
        border.color: adwaita.accent
    }

    Keys.onReturnPressed: {
        clicked()
    }

    Keys.onEnterPressed: {
        clicked()
    }

    Keys.onRightPressed: {
        nextItemInFocusChain(true).forceActiveFocus(Qt.TabFocus)
    }

    Keys.onLeftPressed: {
        nextItemInFocusChain(false).forceActiveFocus(Qt.TabFocus)
    }
}
