import QtQuick 2.9
import QtQuick.Controls 2.2
import QtTest 1.2
import "../../app/gui" as Perigee

Item {
    width: 640
    height: 480

    Button {
        id: toolbar
        text: "Toolbar fixture"
    }

    ListModel {
        id: entries
    }

    Perigee.CenteredGridView {
        id: selectionGrid
        anchors.top: toolbar.bottom
        width: 330
        height: 220
        cellWidth: 100
        cellHeight: 100
        model: entries
        delegate: Perigee.NavigableItemDelegate {
            width: 90
            height: 90
            grid: selectionGrid
            text: label
            onClicked: testCase.launchCount++
            onContextMenuRequested: {
                contextMenu.initiator = this
                contextMenu.open()
            }
        }
    }

    Perigee.NavigableMenu {
        id: contextMenu
        MenuItem {
            text: "Disabled fixture"
            enabled: false
        }
        Perigee.NavigableMenuItem {
            id: menuAction
            text: "Action fixture"
            onTriggered: testCase.actionCount++
        }
    }

    TestCase {
        id: testCase
        name: "GridNavigation"
        when: windowShown
        property int launchCount: 0
        property int actionCount: 0

        function init() {
            contextMenu.close()
            entries.clear()
            for (var entryIndex = 0; entryIndex < 25; entryIndex++) {
                entries.append({label: "Entry " + entryIndex})
            }
            selectionGrid.width = 330
            selectionGrid.height = 220
            selectionGrid.topMargin = 0
            selectionGrid.bottomMargin = 0
            selectionGrid.currentIndex = -1
            selectionGrid.positionViewAtBeginning()
            selectionGrid.forceActiveFocus()
            tryCompare(selectionGrid, "activeFocus", true)
            launchCount = 0
            actionCount = 0
        }

        function cleanup() {
            contextMenu.close()
            compare(launchCount, 0)
        }

        function test_navigation_data() {
            return [
                {tag: "home-no-selection", start: -1, key: Qt.Key_Home, target: 0},
                {tag: "end-no-selection", start: -1, key: Qt.Key_End, target: 24},
                {tag: "page-down-no-selection", start: -1, key: Qt.Key_PageDown, target: 0},
                {tag: "page-up-no-selection", start: -1, key: Qt.Key_PageUp, target: 0},
                {tag: "home", start: 14, key: Qt.Key_Home, target: 0},
                {tag: "end", start: 14, key: Qt.Key_End, target: 24},
                {tag: "page-down-keeps-column", start: 4, key: Qt.Key_PageDown, target: 10},
                {tag: "page-up-keeps-column", start: 10, key: Qt.Key_PageUp, target: 4},
                {tag: "page-down-clamps", start: 22, key: Qt.Key_PageDown, target: 24},
                {tag: "page-up-clamps", start: 2, key: Qt.Key_PageUp, target: 0}
            ]
        }

        function test_navigation(data) {
            selectionGrid.currentIndex = data.start
            keyClick(data.key)
            compare(selectionGrid.currentIndex, data.target)
            verify(selectionGrid.activeFocus)
            tryVerify(function() {
                var selected = selectionGrid.currentItem
                return selected && selected.y >= selectionGrid.contentY &&
                        selected.y + selected.height <= selectionGrid.contentY + selectionGrid.height
            })
        }

        function test_responsivePaging_data() {
            return [
                {tag: "one-column", width: 130, height: 220, target: 2},
                {tag: "one-row", width: 330, height: 110, target: 3},
                {tag: "smaller-than-cell", width: 80, height: 80, target: 1},
                {tag: "wide-grid", width: 530, height: 220, target: 10}
            ]
        }

        function test_responsivePaging(data) {
            selectionGrid.width = data.width
            selectionGrid.height = data.height
            selectionGrid.currentIndex = 0
            keyClick(Qt.Key_PageDown)
            compare(selectionGrid.currentIndex, data.target)
        }

        function test_emptyGrid() {
            entries.clear()
            tryCompare(selectionGrid, "count", 0)
            for (var keyIndex = 0; keyIndex < 4; keyIndex++) {
                keyClick([Qt.Key_Home, Qt.Key_End, Qt.Key_PageUp, Qt.Key_PageDown][keyIndex])
                compare(selectionGrid.currentIndex, -1)
            }
        }

        function test_pageWithVerticalMargins() {
            selectionGrid.topMargin = 20
            selectionGrid.bottomMargin = 5
            selectionGrid.currentIndex = 0
            keyClick(Qt.Key_PageDown)
            compare(selectionGrid.currentIndex, 3)
        }

        function test_modifiedKeys() {
            selectionGrid.currentIndex = 10
            keyClick(Qt.Key_Home, Qt.ControlModifier)
            keyClick(Qt.Key_End, Qt.ShiftModifier)
            keyClick(Qt.Key_PageUp, Qt.AltModifier)
            compare(selectionGrid.currentIndex, 10)
        }

        function test_arrowNavigation() {
            selectionGrid.currentIndex = 0
            keyClick(Qt.Key_Right)
            compare(selectionGrid.currentIndex, 1)
            keyClick(Qt.Key_Down)
            compare(selectionGrid.currentIndex, 4)
            keyClick(Qt.Key_Left)
            compare(selectionGrid.currentIndex, 3)
            keyClick(Qt.Key_Up)
            compare(selectionGrid.currentIndex, 0)
        }

        function test_contextMenu_data() {
            return [
                {tag: "shift-f10", key: Qt.Key_F10, modifiers: Qt.ShiftModifier},
                {tag: "menu-key", key: Qt.Key_Menu, modifiers: Qt.NoModifier}
            ]
        }

        function test_contextMenu(data) {
            selectionGrid.currentIndex = 4
            keyClick(data.key, data.modifiers)
            tryCompare(contextMenu, "opened", true)
            tryCompare(menuAction, "activeFocus", true)
            compare(actionCount, 0)
            keyClick(Qt.Key_Escape)
            tryCompare(contextMenu, "visible", false)
            compare(selectionGrid.currentIndex, 4)
            verify(selectionGrid.activeFocus)
            keyClick(Qt.Key_End)
            compare(selectionGrid.currentIndex, 24)
        }

        function test_contextMenuAction() {
            selectionGrid.currentIndex = 4
            keyClick(Qt.Key_F10, Qt.ShiftModifier)
            tryCompare(contextMenu, "opened", true)
            tryCompare(menuAction, "activeFocus", true)
            keyClick(Qt.Key_Return)
            compare(actionCount, 1)
            tryCompare(contextMenu, "visible", false)
            compare(selectionGrid.currentIndex, 4)
        }

        function test_unmodifiedF10() {
            selectionGrid.currentIndex = 4
            keyClick(Qt.Key_F10)
            verify(!contextMenu.visible)
            compare(selectionGrid.currentIndex, 4)
        }

        function test_toolbarFocus() {
            selectionGrid.currentIndex = 4
            toolbar.forceActiveFocus()
            keyClick(Qt.Key_End)
            compare(selectionGrid.currentIndex, 4)
            verify(toolbar.activeFocus)
        }
    }
}
