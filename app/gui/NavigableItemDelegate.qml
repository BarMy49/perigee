import QtQuick 2.0
import QtQuick.Controls 2.2

ItemDelegate {
    property GridView grid
    signal contextMenuRequested()

    highlighted: grid.activeFocus && grid.currentItem === this

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
