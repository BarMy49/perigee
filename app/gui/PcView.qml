import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3

import ComputerModel 1.0

import ComputerManager 1.0
import StreamingPreferences 1.0
import SystemProperties 1.0
import SdlGamepadKeyNavigation 1.0

CenteredGridView {
    property ComputerModel computerModel : createModel()

    id: pcGrid
    focus: true
    activeFocusOnTab: true
    topMargin: 20
    bottomMargin: 88
    cellWidth: 272; cellHeight: 252;
    objectName: qsTr("Computers")

    Component.onCompleted: {
        // Don't show any highlighted item until interacting with them.
        // We do this here instead of onActivated to avoid losing the user's
        // selection when backing out of a different page of the app.
        currentIndex = -1
    }

    // Note: Any initialization done here that is critical for streaming must
    // also be done in CliStartStreamSegue.qml, since this code does not run
    // for command-line initiated streams.
    StackView.onActivated: {
        // Setup signals on CM
        ComputerManager.computerAddCompleted.connect(addComplete)

        // Highlight the first item if a gamepad is connected
        if (currentIndex === -1 && SdlGamepadKeyNavigation.getConnectedGamepads() > 0) {
            currentIndex = 0
        }
    }

    StackView.onDeactivating: {
        ComputerManager.computerAddCompleted.disconnect(addComplete)
    }

    function pairingComplete(error)
    {
        // Close the PIN dialog
        pairDialog.close()

        // Display a failed dialog if we got an error
        if (error !== undefined) {
            errorDialog.text = error
            errorDialog.helpText = ""
            errorDialog.open()
        }
    }

    function addComplete(success, detectedPortBlocking)
    {
        if (!success) {
            errorDialog.text = qsTr("Unable to connect to the specified PC.")

            if (detectedPortBlocking) {
                errorDialog.text += "\n\n" + qsTr("This PC's Internet connection is blocking Perigee. Streaming over the Internet may not work while connected to this network.")
            }
            else {
                errorDialog.helpText = qsTr("Click the Help button for possible solutions.")
            }

            errorDialog.open()
        }
    }

    function createModel()
    {
        var model = Qt.createQmlObject('import ComputerModel 1.0; ComputerModel {}', parent, '')
        model.initialize(ComputerManager)
        model.pairingCompleted.connect(pairingComplete)
        model.connectionTestCompleted.connect(testConnectionDialog.connectionTestComplete)
        return model
    }

    AdwaitaPalette {
        id: adwaita
        dark: pcGrid.Material.theme === Material.Dark
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(pcGrid.width - 48, 520)
        spacing: 16
        visible: pcGrid.count === 0

        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 96
            height: 96

            ThemedImage {
                anchors.fill: parent
                iconSource: "qrc:/res/desktop_windows-48px.svg"
                sourceSize {
                    width: 96
                    height: 96
                }
                opacity: 0.35
            }

            BusyIndicator {
                id: searchSpinner
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -8
                width: 40
                height: 40
                visible: StreamingPreferences.enableMdns
                running: visible
            }
        }

        Label {
            width: parent.width
            text: StreamingPreferences.enableMdns ? qsTr("Searching for compatible hosts on your local network...")
                                                  : qsTr("Automatic PC discovery is disabled. Add your PC manually.")
            font.pointSize: 15
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }

        Label {
            width: parent.width
            text: qsTr("Make sure your host PC is turned on, running Sunshine or Apollo, and connected to the same network as this device.")
            color: adwaita.dimForeground
            font.pointSize: 11
            lineHeight: 1.2
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }

        Button {
            id: emptyAddPcButton
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTranslate("main", "Add PC manually")
            focusPolicy: Qt.StrongFocus
            leftPadding: 24
            rightPadding: 24
            topPadding: 10
            bottomPadding: 10
            font.bold: true
            onClicked: addPcDialog.open()

            contentItem: Label {
                text: emptyAddPcButton.text
                font: emptyAddPcButton.font
                color: adwaita.accentForeground
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                implicitHeight: 40
                radius: height / 2
                color: emptyAddPcButton.down ? Qt.darker(adwaita.accent, 1.2) :
                       emptyAddPcButton.hovered ? Qt.lighter(adwaita.accent, 1.08) : adwaita.accent
                border.width: emptyAddPcButton.visualFocus ? 3 : 0
                border.color: adwaita.dark ? "#99c1f1" : "#1a5fb4"
            }
        }
    }

    model: computerModel

    delegate: NavigableItemDelegate {
        width: 260; height: 240;
        grid: pcGrid

        property alias pcContextMenu : pcContextMenuLoader.item

        ThemedImage {
            id: pcIcon
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
            iconSource: "qrc:/res/desktop_windows-48px.svg"
            opacity: model.online || model.statusUnknown ? 0.85 : 0.5
            sourceSize {
                width: 120
                height: 120
            }
        }

        ThemedImage {
            id: stateIcon
            anchors.horizontalCenter: pcIcon.horizontalCenter
            anchors.verticalCenter: pcIcon.verticalCenter
            anchors.verticalCenterOffset: -10
            visible: !model.statusUnknown && (!model.online || !model.paired)
            iconSource: !model.online ? "qrc:/res/warning_FILL1_wght300_GRAD200_opsz24.svg" : "qrc:/res/baseline-lock-24px.svg"
            sourceSize {
                width: !model.online ? 46 : 42
                height: !model.online ? 46 : 42
            }
        }

        BusyIndicator {
            id: statusUnknownSpinner
            anchors.horizontalCenter: pcIcon.horizontalCenter
            anchors.verticalCenter: pcIcon.verticalCenter
            anchors.verticalCenterOffset: -9
            width: 46
            height: 46
            visible: model.statusUnknown
            running: visible
        }

        Label {
            id: pcNameText
            text: model.name

            width: parent.width
            anchors.top: pcIcon.bottom
            anchors.topMargin: 8
            leftPadding: 16
            rightPadding: 16
            font.pointSize: 15
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 1
            elide: Text.ElideRight
        }

        Row {
            id: pcStatusRow
            anchors.top: pcNameText.bottom
            anchors.topMargin: 6
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 6

            readonly property color statusColor: model.statusUnknown ? adwaita.neutralForeground :
                                                 !model.online ? adwaita.neutralForeground :
                                                 !model.paired ? adwaita.warningForeground : adwaita.successForeground
            readonly property string statusText: model.statusUnknown ? qsTranslate("ComputerModel", "Unknown") :
                                                 !model.online ? qsTr("Offline") :
                                                 !model.paired ? qsTranslate("ComputerModel", "Unpaired") : qsTr("Online")

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: 8
                height: 8
                radius: 4
                color: pcStatusRow.statusColor
            }

            Label {
                text: pcStatusRow.statusText
                color: adwaita.dimForeground
                font.pointSize: 11
            }
        }

        Accessible.name: model.name + ", " + pcStatusRow.statusText

        // Display a tooltip with the full name if it's truncated
        ToolTip.text: model.name
        ToolTip.delay: 1000
        ToolTip.timeout: 5000
        ToolTip.visible: (hovered || highlighted) && pcNameText.truncated

        Loader {
            id: pcContextMenuLoader
            asynchronous: true
            sourceComponent: NavigableMenu {
                id: pcContextMenu
                initiator: pcContextMenuLoader.parent
                MenuItem {
                    text: qsTr("PC status: %1").arg(model.online ? qsTr("Online") : qsTr("Offline"))
                    font.bold: true
                    enabled: false
                }
                NavigableMenuItem {
                    text: qsTr("View all apps")
                    onTriggered: {
                        var component = Qt.createComponent("AppView.qml")
                        var appView = component.createObject(stackView, {"computerIndex": index, "objectName": model.name, "showHiddenGames": true})
                        stackView.push(appView)
                    }
                    visible: model.online && model.paired
                }
                NavigableMenuItem {
                    text: qsTr("Wake PC")
                    onTriggered: computerModel.wakeComputer(index)
                    visible: !model.online && model.wakeable
                }
                NavigableMenuItem {
                    text: qsTr("Test network")
                    onTriggered: {
                        computerModel.testConnectionForComputer(index)
                        testConnectionDialog.open()
                    }
                }

                NavigableMenuItem {
                    text: qsTr("Rename PC")
                    onTriggered: {
                        renamePcDialog.pcIndex = index
                        renamePcDialog.originalName = model.name
                        renamePcDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Delete PC")
                    onTriggered: {
                        deletePcDialog.pcIndex = index
                        deletePcDialog.pcName = model.name
                        deletePcDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("View details")
                    onTriggered: {
                        showPcDetailsDialog.pcDetails = model.details
                        showPcDetailsDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: model.clipboardEnabled ? qsTr("Disable session text clipboard") : qsTr("Enable session text clipboard")
                    onTriggered: {
                        if (model.clipboardEnabled) computerModel.setClipboardEnabled(model.hostUuid, false)
                        else {
                            clipboardConsent.hostUuid = model.hostUuid
                            clipboardConsent.open()
                        }
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Host capabilities")
                    visible: model.capabilities.advertised
                    onTriggered: {
                        capabilityDialog.details = model.capabilityDetails
                        capabilityDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Display launch options")
                    onTriggered: {
                        displayLaunchDialog.hostUuid = model.hostUuid
                        displayLaunchDialog.details = model.displayLaunchDetails
                        displayLaunchDialog.available = model.displayLaunchAvailable
                        virtualDisplayCheck.checked = model.requestVirtualDisplay
                        displayLaunchDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Use application UUID")
                    checkable: true
                    checked: model.useAppUuid
                    enabled: model.useAppUuid || model.appUuidAvailable
                    onTriggered: computerModel.setUseAppUuid(model.hostUuid, !model.useAppUuid)
                }
                NavigableMenuItem {
                    text: qsTr("Configure SFTP")
                    onTriggered: {
                        transferDialog.hostUuid = model.hostUuid
                        transferText.text = model.transferUrl
                        transferDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Open SFTP")
                    visible: model.transferUrl.length > 0
                    onTriggered: {
                        if (!computerModel.openTransferUrl(model.hostUuid)) transferOpenError.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Configure administration link")
                    onTriggered: {
                        administrationDialog.hostUuid = model.hostUuid
                        administrationText.text = model.administrationUrl
                        administrationDialog.open()
                    }
                }
                NavigableMenuItem {
                    text: qsTr("Open administration")
                    visible: model.administrationUrl.length > 0 && SystemProperties.hasBrowser
                    onTriggered: Qt.openUrlExternally(model.administrationUrl)
                }
            }
        }

        onClicked: {
            if (model.online) {
                if (!model.serverSupported) {
                    errorDialog.text = qsTr("The version of GeForce Experience on %1 is not supported by this build of Perigee. You must update Perigee to stream from %1.").arg(model.name)
                    errorDialog.helpText = ""
                    errorDialog.open()
                }
                else if (model.paired) {
                    // go to game view
                    var component = Qt.createComponent("AppView.qml")
                    var appView = component.createObject(stackView, {"computerIndex": index, "objectName": model.name})
                    stackView.push(appView)
                }
                else {
                    var pin = computerModel.generatePinString()

                    // Kick off pairing in the background
                    computerModel.pairComputer(index, pin)

                    // Display the pairing dialog
                    pairDialog.pin = pin
                    pairDialog.open()
                }
            } else if (!model.online) {
                // Using open() here because it may be activated by keyboard
                pcContextMenu.open()
            }
        }

        onPressAndHold: {
            // popup() ensures the menu appears under the mouse cursor
            if (pcContextMenu.popup) {
                pcContextMenu.popup()
            }
            else {
                // Qt 5.9 doesn't have popup()
                pcContextMenu.open()
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton;
            onClicked: {
                parent.pressAndHold()
            }
        }

        onContextMenuRequested: {
            // We must use open() here so the menu is positioned on
            // the ItemDelegate and not where the mouse cursor is
            pcContextMenu.open()
        }

        Keys.onDeletePressed: {
            deletePcDialog.pcIndex = index
            deletePcDialog.pcName = model.name
            deletePcDialog.open()
        }
    }

    NavigableDialog {
        id: displayLaunchDialog
        property string hostUuid: ""
        property string details: ""
        property bool available: false
        title: qsTr("Display launch options")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: {
            if (!computerModel.setRequestVirtualDisplay(hostUuid, virtualDisplayCheck.checked)) displayLaunchSaveError.open()
        }
        ColumnLayout {
            width: 420
            Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: displayLaunchDialog.details
            }
            CheckBox {
                id: virtualDisplayCheck
                text: qsTr("Request a virtual display for new launches")
                enabled: checked || displayLaunchDialog.available
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: qsTr("Uses your stream resolution and frame rate. This requests a display; it does not confirm the host display mode. Applies to a new launch, not resume or an active session. Host display defaults may also create a virtual display when this option is off. Live monitor changes and fractional refresh are unavailable.")
            }
        }
    }

    NavigableDialog {
        id: displayLaunchSaveError
        title: qsTr("Display options not saved")
        standardButtons: Dialog.Ok
        Label {
            width: 420
            wrapMode: Text.Wrap
            text: qsTr("Host availability changed. Reopen display launch options to check current permissions and driver status.")
        }
    }

    NavigableDialog {
        id: clipboardConsent
        property string hostUuid: ""
        title: qsTr("Session text clipboard")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: computerModel.setClipboardEnabled(hostUuid, true)
        Label {
            width: 420
            wrapMode: Text.Wrap
            text: qsTr("Allow text clipboard transfer for this host during a stream. Requires host clipboard permissions. On GNOME Wayland, text changes synchronize automatically while the stream is focused; existing text is not transferred at connection. Other desktops use manual transfer. Ctrl+Alt+Shift+J sends local text; Ctrl+Alt+Shift+K receives remote text. Slow or unavailable clipboard reads switch to manual transfer. Changes apply to the next stream.")
        }
    }

    NavigableDialog {
        id: capabilityDialog
        property string details: ""
        title: qsTr("Host capabilities")
        standardButtons: Dialog.Ok

        ScrollView {
            id: capabilityScroll
            width: 480
            height: 360
            contentWidth: availableWidth
            clip: true
            TextArea {
                width: capabilityScroll.availableWidth
                text: capabilityDialog.details
                textFormat: TextEdit.PlainText
                readOnly: true
                selectByMouse: true
                wrapMode: TextEdit.Wrap
            }
        }
    }

    NavigableMessageDialog {
        id: transferOpenError
        objectName: "transferOpenError"
        background: Rectangle {
            color: transferOpenError.Material.theme === Material.Light ? "#fafafa" : "#303030"
            radius: 8
        }
        text: qsTr("Unable to open SFTP. Check your desktop handler.")
        standardButtons: Dialog.Ok
    }

    NavigableDialog {
        id: transferDialog
        objectName: "transferDialog"
        width: Math.min(480, pcGrid.width - 32)
        background: Rectangle {
            color: transferDialog.Material.theme === Material.Light ? "#fafafa" : "#303030"
            radius: 8
        }
        property string hostUuid: ""
        title: qsTr("Configure SFTP")
        standardButtons: Dialog.Cancel
        onOpened: {
            transferError.visible = false
            transferText.forceActiveFocus()
        }
        ColumnLayout {
            width: transferDialog.availableWidth
            Label {
                text: qsTr("Use an existing SSH service. Enter an sftp:// URL without login credentials. Leave empty to remove it.")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            TextField {
                id: transferText
                objectName: "transferText"
                Keys.onReturnPressed: saveTransferButton.clicked()
                Keys.onEnterPressed: saveTransferButton.clicked()
                placeholderText: "sftp://host/path"
                selectByMouse: true
                Layout.fillWidth: true
            }
            Label {
                id: transferError
                objectName: "transferError"
                visible: false
                text: qsTr("Enter a valid SFTP URL without credentials, query parameters, or a fragment.")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
        }
        footer: DialogButtonBox {
            standardButtons: transferDialog.standardButtons
            Button {
                id: saveTransferButton
                objectName: "saveTransferUrl"
                text: qsTr("Save")
                DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
                onClicked: {
                    if (computerModel.setTransferUrl(transferDialog.hostUuid, transferText.text)) transferDialog.accept()
                    else transferError.visible = true
                }
            }
            onRejected: transferDialog.reject()
        }
    }

    NavigableDialog {
        id: administrationDialog
        property string hostUuid: ""
        title: qsTr("Host administration link")
        standardButtons: Dialog.Ok | Dialog.Cancel

        function saveLink() {
            if (computerModel.setAdministrationUrl(hostUuid, administrationText.text)) {
                accept()
            }
            else {
                administrationError.visible = true
                administrationText.forceActiveFocus()
            }
        }
        onOpened: {
            administrationError.visible = false
            administrationText.forceActiveFocus()
        }
        ColumnLayout {
            width: 420
            Label {
                text: qsTr("Enter the host's HTTPS administration URL. Login remains in your browser. Leave empty to remove the link.")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            TextField {
                id: administrationText
                placeholderText: "https://host:port/"
                Layout.fillWidth: true
                selectByMouse: true
                Keys.onReturnPressed: administrationDialog.saveLink()
                Keys.onEnterPressed: administrationDialog.saveLink()
            }
            Label {
                id: administrationError
                visible: false
                text: qsTr("Enter a valid HTTPS URL without login credentials, query parameters, or a fragment.")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
        }
        footer: DialogButtonBox {
            standardButtons: administrationDialog.standardButtons
            onAccepted: administrationDialog.saveLink()
            onRejected: administrationDialog.reject()
        }
    }

    ErrorMessageDialog {
        id: errorDialog

        // Using Setup-Guide here instead of Troubleshooting because it's likely that users
        // will arrive here by forgetting to enable GameStream or not forwarding ports.
        helpUrl: "https://github.com/moonlight-stream/moonlight-docs/wiki/Setup-Guide"
    }

    NavigableMessageDialog {
        id: pairDialog
        closePolicy: Popup.CloseOnEscape

        // don't allow edits to the rest of the window while open
        property string pin : "0000"
        text:qsTr("Please enter %1 on your host PC. This dialog will close when pairing is completed.").arg(pin)+"\n\n"+
             qsTr("If your host PC is running Sunshine, navigate to the Sunshine web UI to enter the PIN.")
        standardButtons: Dialog.Cancel
        onRejected: {
            // FIXME: We should interrupt pairing here
        }
    }

    NavigableMessageDialog {
        id: deletePcDialog
        // don't allow edits to the rest of the window while open
        property int pcIndex : -1
        property string pcName : ""
        text: qsTr("Are you sure you want to remove '%1'?").arg(pcName)
        standardButtons: Dialog.Yes | Dialog.No

        onAccepted: {
            computerModel.deleteComputer(pcIndex)
        }
    }

    NavigableMessageDialog {
        id: testConnectionDialog
        closePolicy: Popup.CloseOnEscape
        standardButtons: Dialog.Ok

        onAboutToShow: {
            testConnectionDialog.text = qsTr("Perigee is testing your network connection to determine if any required ports are blocked.") + "\n\n" + qsTr("This may take a few seconds…")
            showSpinner = true
        }

        function connectionTestComplete(result, blockedPorts)
        {
            if (result === -1) {
                text = qsTr("The network test could not be performed because none of the upstream Moonlight connection testing servers were reachable from this PC. Check your Internet connection or try again later.")
                imageSrc = "qrc:/res/baseline-warning-24px.svg"
            }
            else if (result === 0) {
                text = qsTr("This network does not appear to be blocking Perigee. If you still have trouble connecting, check your PC's firewall settings.") + "\n\n" + qsTr("If you are trying to stream over the Internet, install the Moonlight Internet Hosting Tool on your gaming PC and run the included Internet Streaming Tester to check your gaming PC's Internet connection.")
                imageSrc = "qrc:/res/baseline-check_circle_outline-24px.svg"
            }
            else {
                text = qsTr("Your PC's current network connection seems to be blocking Perigee. Streaming over the Internet may not work while connected to this network.") + "\n\n" + qsTr("The following network ports were blocked:") + "\n"
                text += blockedPorts
                imageSrc = "qrc:/res/baseline-error_outline-24px.svg"
            }

            // Stop showing the spinner and show the image instead
            showSpinner = false
        }
    }

    NavigableDialog {
        id: renamePcDialog
        property string label: qsTr("Enter the new name for this PC:")
        property string originalName
        property int pcIndex : -1;

        standardButtons: Dialog.Ok | Dialog.Cancel

        onOpened: {
            // Force keyboard focus on the textbox so keyboard navigation works
            editText.forceActiveFocus()
        }

        onClosed: {
            editText.clear()
        }

        onAccepted: {
            if (editText.text) {
                computerModel.renameComputer(pcIndex, editText.text)
            }
        }

        ColumnLayout {
            Label {
                text: renamePcDialog.label
                font.bold: true
            }

            TextField {
                id: editText
                placeholderText: renamePcDialog.originalName
                Layout.fillWidth: true
                focus: true

                Keys.onReturnPressed: {
                    renamePcDialog.accept()
                }

                Keys.onEnterPressed: {
                    renamePcDialog.accept()
                }
            }
        }
    }

    NavigableMessageDialog {
        id: showPcDetailsDialog
        property string pcDetails : "";
        text: showPcDetailsDialog.pcDetails
        imageSrc: "qrc:/res/baseline-help_outline-24px.svg"
        standardButtons: Dialog.Ok
    }

    ScrollBar.vertical: ScrollBar {}
}
