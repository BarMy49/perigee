import QtQuick 2.9

QtObject {
    property bool dark: true

    readonly property color accent: "#3584e4"
    readonly property color accentForeground: "#ffffff"
    readonly property color windowBackground: dark ? "#222226" : "#fafafb"
    readonly property color windowForeground: dark ? "#ffffff" : "#333337"
    readonly property color headerBackground: dark ? "#2e2e32" : "#ffffff"
    readonly property color headerShade: dark ? "#16161a" : "#e0e0e3"
    readonly property color cardBackground: dark ? "#343438" : "#ffffff"
    readonly property color cardHover: dark ? "#3d3d41" : "#f3f3f5"
    readonly property color cardPressed: dark ? "#46464a" : "#e8e8eb"
    readonly property color cardBorder: dark ? "#3a3a3e" : "#e3e3e6"
    readonly property color buttonHover: dark ? "#1affffff" : "#12000006"
    readonly property color buttonPressed: dark ? "#33ffffff" : "#24000006"
}
