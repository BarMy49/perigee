import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

GroupBox {
    id: preferencesGroup
    spacing: 8

    AdwaitaPalette {
        id: adwaita
        dark: preferencesGroup.Material.theme === Material.Dark
    }

    label: Label {
        x: 2
        width: preferencesGroup.availableWidth
        text: preferencesGroup.title
        font.pointSize: 11
        font.bold: true
        elide: Text.ElideRight
    }

    background: Rectangle {
        y: preferencesGroup.topPadding - preferencesGroup.padding
        width: parent.width
        height: parent.height - preferencesGroup.topPadding + preferencesGroup.padding
        radius: 12
        color: adwaita.cardBackground
        border.width: 1
        border.color: adwaita.cardBorder
    }
}
