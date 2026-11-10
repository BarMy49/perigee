import QtQuick 2.9
import "../../app/gui" as Perigee

Item {
    width: 760
    height: 630
    Perigee.AppView {
        objectName: "appGrid"
        anchors.fill: parent
        computerIndex: 0
        showGames: true
    }
}
