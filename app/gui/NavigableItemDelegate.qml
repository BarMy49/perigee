import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

ItemDelegate {
    id: itemDelegate
    property GridView grid
    signal contextMenuRequested()

    highlighted: grid.activeFocus && grid.currentItem === this

    AdwaitaPalette {
        id: adwaita
        dark: itemDelegate.Material.theme === Material.Dark
    }

    background: Rectangle {
        radius: 12
        color: itemDelegate.down ? adwaita.cardPressed : itemDelegate.hovered ? adwaita.cardHover : adwaita.cardBackground
        border.width: itemDelegate.highlighted || itemDelegate.visualFocus ? 2 : 1
        border.color: itemDelegate.highlighted || itemDelegate.visualFocus ? adwaita.accent : adwaita.cardBorder
    }

    Keys.onMenuPressed: contextMenuRequested()
    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_F10 && event.modifiers === Qt.ShiftModifier) {
            contextMenuRequested()
            event.accepted = true
        }
        else {
            event.accepted = false
        }
    }

    Keys.onLeftPressed: {
        grid.moveCurrentIndexLeft()
    }
    Keys.onRightPressed: {
        grid.moveCurrentIndexRight()
    }
    Keys.onDownPressed: {
        grid.moveCurrentIndexDown()
    }
    Keys.onUpPressed: {
        grid.moveCurrentIndexUp()

        // If we've reached the top of the grid, move focus to the toolbar
        if (grid.currentItem === this) {
            nextItemInFocusChain(false).forceActiveFocus(Qt.TabFocus)
        }
    }
    Keys.onReturnPressed: {
        clicked()
    }
    Keys.onEnterPressed: {
        clicked()
    }
}
