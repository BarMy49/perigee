import QtQuick 2.9
import QtQuick.Controls 2.2

GridView {
    property int minMargin: 10
    property real availableWidth: (parent.width - 2 * minMargin)
    property int itemsPerRow: availableWidth / cellWidth
    property real horizontalMargin: itemsPerRow < count && availableWidth >= cellWidth ?
                                        (availableWidth % cellWidth) / 2 : minMargin

    function updateMargins() {
        leftMargin = horizontalMargin
        rightMargin = horizontalMargin
    }

    onHorizontalMarginChanged: {
        updateMargins()
    }

    Component.onCompleted: {
        updateMargins()
    }

    Keys.onPressed: function(event) {
        if (event.modifiers !== Qt.NoModifier || count === 0) {
            event.accepted = false
            return
        }

        var targetIndex
        var pageSize = Math.max(1, Math.floor((width - leftMargin - rightMargin) / cellWidth)) *
                Math.max(1, Math.floor((height - topMargin - bottomMargin) / cellHeight))
        switch (event.key) {
        case Qt.Key_Home:
            targetIndex = 0
            break
        case Qt.Key_End:
            targetIndex = count - 1
            break
        case Qt.Key_PageUp:
            targetIndex = Math.max(0, currentIndex - pageSize)
            break
        case Qt.Key_PageDown:
            targetIndex = currentIndex < 0 ? 0 : Math.min(count - 1, currentIndex + pageSize)
            break
        default:
            event.accepted = false
            return
        }

        currentIndex = targetIndex
        positionViewAtIndex(currentIndex, GridView.Contain)
        event.accepted = true
    }

    boundsBehavior: Flickable.OvershootBounds
}
