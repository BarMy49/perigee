import QtQuick 2.9
import "../../app/gui" as Perigee

Item {
    width: 500
    height: 400
    Perigee.CenteredGridView {
        objectName: "selectionGrid"
        anchors.fill: parent
        cellWidth: 100
        cellHeight: 100
        model: favoritesModel
        focus: true
        delegate: Perigee.NavigableItemDelegate {
            width: 90
            height: 90
            grid: GridView.view
            text: model.name
            property int selectedAppId: model.appid
        }
    }
}
