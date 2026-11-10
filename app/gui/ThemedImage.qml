import QtQuick 2.0
import QtQuick.Controls.Material 2.2

Image {
    property url iconSource
    source: Material.theme === Material.Light ? iconSource.toString().replace(".svg", "-light.svg") : iconSource
}
