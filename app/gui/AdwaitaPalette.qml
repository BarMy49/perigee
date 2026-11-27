import QtQuick 2.9

QtObject {
    property bool dark: true

    readonly property color accent: "#3584e4"
    readonly property color accentForeground: "#ffffff"
    readonly property color windowBackground: dark ? "#222226" : "#fafafb"
    readonly property color windowForeground: dark ? "#ffffff" : "#2e2e32"
    readonly property color dimForeground: dark ? "#c0bfc4" : "#5e5c64"
    readonly property color headerBackground: dark ? "#2e2e32" : "#ffffff"
    readonly property color headerShade: dark ? "#16161a" : "#e0e0e3"
    readonly property color cardBackground: dark ? "#343438" : "#ffffff"
    readonly property color cardHover: dark ? "#3d3d41" : "#f3f3f5"
    readonly property color cardPressed: dark ? "#46464a" : "#e8e8eb"
    readonly property color cardBorder: dark ? "#3a3a3e" : "#e3e3e6"
    readonly property color buttonHover: dark ? "#1affffff" : "#12000006"
    readonly property color successForeground: dark ? "#8ff0a4" : "#1b7a4c"
    readonly property color warningForeground: dark ? "#f8e45c" : "#8a5d00"
    readonly property color neutralForeground: dark ? "#9a9996" : "#77767b"
    readonly property color buttonPressed: dark ? "#33ffffff" : "#24000006"
}
