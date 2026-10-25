#include "computermodel.h"
#include "backend/hostadministration.h"

#include <QThreadPool>

ComputerModel::ComputerModel(QObject* object)
    : QAbstractListModel(object) {}

void ComputerModel::initialize(ComputerManager* computerManager)
{
    m_ComputerManager = computerManager;
    connect(m_ComputerManager, &ComputerManager::computerStateChanged,
            this, &ComputerModel::handleComputerStateChanged);
    connect(m_ComputerManager, &ComputerManager::pairingCompleted,
            this, &ComputerModel::handlePairingCompleted);

    m_Computers = m_ComputerManager->getComputers();
}

QVariant ComputerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Q_ASSERT(index.row() < m_Computers.count());

    NvComputer* computer = m_Computers[index.row()];
    QReadLocker lock(&computer->lock);

    switch (role) {
    case ClipboardEnabledRole:
        return computer->clipboardEnabled;
    case HostUuidRole:
        return computer->uuid;
    case AdministrationUrlRole:
        return computer->administrationUrl;
    case CapabilitiesRole:
        return computer->apolloCapabilities.toVariantMap();
    case CapabilityDetailsRole: {
        using State = ApolloCapabilities::State;
        const auto& capabilities = computer->apolloCapabilities;
        const auto stateText = [](State state) {
            switch (state) {
            case State::Absent: return tr("Not advertised");
            case State::Invalid: return tr("Invalid metadata");
            case State::Unauthenticated: return tr("No current authenticated metadata");
            case State::Available: return tr("Allowed");
            case State::Denied: return tr("Denied");
            }
            return tr("Unknown");
        };
        QStringList lines = {tr("Optional Apollo-compatible fields; this does not identify the host product or version."),
                             tr("Metadata snapshot. Reopen this panel to refresh. Availability does not grant permissions."),
                             tr("Permission metadata: %1").arg(capabilities.permissionsState == State::Available ? tr("Authenticated mask") : stateText(capabilities.permissionsState))};
        const struct PermissionLabel {
            const char* label;
            quint32 mask;
        } labels[] = {
            {QT_TR_NOOP("Keyboard"), ApolloCapabilities::Keyboard}, {QT_TR_NOOP("Mouse"), ApolloCapabilities::Mouse},
            {QT_TR_NOOP("Controller"), ApolloCapabilities::Controller}, {QT_TR_NOOP("Touch"), ApolloCapabilities::Touch},
            {QT_TR_NOOP("Pen"), ApolloCapabilities::Pen}, {QT_TR_NOOP("Clipboard send"), ApolloCapabilities::ClipboardSet},
            {QT_TR_NOOP("Clipboard receive"), ApolloCapabilities::ClipboardRead}, {QT_TR_NOOP("Server commands"), ApolloCapabilities::ServerCommand},
            {QT_TR_NOOP("List applications"), ApolloCapabilities::List}, {QT_TR_NOOP("View"), ApolloCapabilities::View},
            {QT_TR_NOOP("Launch"), ApolloCapabilities::Launch}
        };
        for (const auto& label : labels) {
            lines.append(tr("%1: %2").arg(tr(label.label), stateText(capabilities.permission(label.mask))));
        }
        if (capabilities.permissionsState == State::Available || capabilities.permissionsState == State::Denied) {
            lines.append(tr("Permission mask: 0x%1").arg(QString::number(capabilities.permissionMask, 16)));
        }
        const auto displayText = [&stateText](State state, bool value) {
            return state == State::Available ? (value ? tr("Yes (host reported)") : tr("No (host reported)")) : stateText(state);
        };
        lines.append(tr("Virtual-display capable: %1").arg(displayText(capabilities.displayCapableState, capabilities.displayCapable)));
        lines.append(tr("Virtual-display driver ready: %1").arg(displayText(capabilities.displayDriverState, capabilities.displayDriverReady)));
        lines.append(tr("Advertised command list: %1").arg(stateText(capabilities.commandsState)));
        for (int commandIndex = 0; commandIndex < capabilities.commandNames.size(); commandIndex++) {
            lines.append(tr("%1: %2").arg(commandIndex).arg(capabilities.commandNames.at(commandIndex)));
        }
        return lines.join('\n');
    }

    case NameRole:
        return computer->name;
    case OnlineRole:
        return computer->state == NvComputer::CS_ONLINE;
    case PairedRole:
        return computer->pairState == NvComputer::PS_PAIRED;
    case BusyRole:
        return computer->currentGameId != 0;
    case WakeableRole:
        return !computer->macAddress.isEmpty();
    case StatusUnknownRole:
        return computer->state == NvComputer::CS_UNKNOWN;
    case ServerSupportedRole:
        return computer->isSupportedServerVersion;
    case DetailsRole: {
        QString state, pairState;

        switch (computer->state) {
        case NvComputer::CS_ONLINE:
            state = tr("Online");
            break;
        case NvComputer::CS_OFFLINE:
            state = tr("Offline");
            break;
        default:
            state = tr("Unknown");
            break;
        }

        switch (computer->pairState) {
        case NvComputer::PS_PAIRED:
            pairState = tr("Paired");
            break;
        case NvComputer::PS_NOT_PAIRED:
            pairState = tr("Unpaired");
            break;
        default:
            pairState = tr("Unknown");
            break;
        }

        return tr("Name: %1").arg(computer->name) + '\n' +
               tr("Status: %1").arg(state) + '\n' +
               tr("Active Address: %1").arg(computer->activeAddress.toString()) + '\n' +
               tr("UUID: %1").arg(computer->uuid) + '\n' +
               tr("Local Address: %1").arg(computer->localAddress.toString()) + '\n' +
               tr("Remote Address: %1").arg(computer->remoteAddress.toString()) + '\n' +
               tr("IPv6 Address: %1").arg(computer->ipv6Address.toString()) + '\n' +
               tr("Manual Address: %1").arg(computer->manualAddress.toString()) + '\n' +
               tr("MAC Address: %1").arg(computer->macAddress.isEmpty() ? tr("Unknown") : QString(computer->macAddress.toHex(':'))) + '\n' +
               tr("Pair State: %1").arg(pairState) + '\n' +
               tr("Running Game ID: %1").arg(computer->state == NvComputer::CS_ONLINE ? QString::number(computer->currentGameId) : tr("Unknown")) + '\n' +
               tr("HTTPS Port: %1").arg(computer->state == NvComputer::CS_ONLINE ? QString::number(computer->activeHttpsPort) : tr("Unknown"));
    }
    default:
        return QVariant();
    }
}

bool ComputerModel::setClipboardEnabled(QString hostUuid, bool enabled)
{
    for (NvComputer* computer : std::as_const(m_Computers)) {
        {
            QWriteLocker lock(&computer->lock);
            if (computer->uuid != hostUuid) continue;
            computer->clipboardEnabled = enabled;
        }
        m_ComputerManager->clientSideAttributeUpdated(computer);
        return true;
    }
    return false;
}

bool ComputerModel::setAdministrationUrl(QString hostUuid, QString text)
{
    const QUrl url = HostAdministration::validatedUrl(text);
    if (!text.trimmed().isEmpty() && url.isEmpty()) {
        return false;
    }
    for (NvComputer* computer : std::as_const(m_Computers)) {
        {
            QWriteLocker lock(&computer->lock);
            if (computer->uuid != hostUuid) {
                continue;
            }
            computer->administrationUrl = url.toString(QUrl::FullyEncoded);
        }
        m_ComputerManager->clientSideAttributeUpdated(computer);
        return true;
    }
    return false;
}

int ComputerModel::rowCount(const QModelIndex& parent) const
{
    // We should not return a count for valid index values,
    // only the parent (which will not have a "valid" index).
    if (parent.isValid()) {
        return 0;
    }

    return m_Computers.count();
}

QHash<int, QByteArray> ComputerModel::roleNames() const
{
    QHash<int, QByteArray> names;

    names[NameRole] = "name";
    names[OnlineRole] = "online";
    names[PairedRole] = "paired";
    names[BusyRole] = "busy";
    names[WakeableRole] = "wakeable";
    names[StatusUnknownRole] = "statusUnknown";
    names[ServerSupportedRole] = "serverSupported";
    names[DetailsRole] = "details";
    names[CapabilitiesRole] = "capabilities";
    names[CapabilityDetailsRole] = "capabilityDetails";
    names[AdministrationUrlRole] = "administrationUrl";
    names[ClipboardEnabledRole] = "clipboardEnabled";
    names[HostUuidRole] = "hostUuid";

    return names;
}

Session* ComputerModel::createSessionForCurrentGame(int computerIndex)
{
    Q_ASSERT(computerIndex < m_Computers.count());

    NvComputer* computer = m_Computers[computerIndex];

    // We must currently be streaming a game to use this function
    Q_ASSERT(computer->currentGameId != 0);

    for (NvApp& app : computer->appList) {
        if (app.id == computer->currentGameId) {
            return new Session(computer, app);
        }
    }

    // We have a current running app but it's not in our app list
    Q_ASSERT(false);
    return nullptr;
}

void ComputerModel::deleteComputer(int computerIndex)
{
    Q_ASSERT(computerIndex < m_Computers.count());

    beginRemoveRows(QModelIndex(), computerIndex, computerIndex);

    // m_Computer[computerIndex] will be deleted by this call
    m_ComputerManager->deleteHost(m_Computers[computerIndex]);

    // Remove the now invalid item
    m_Computers.removeAt(computerIndex);

    endRemoveRows();
}

class DeferredWakeHostTask : public QRunnable
{
public:
    DeferredWakeHostTask(NvComputer* computer)
        : m_Computer(computer) {}

    void run()
    {
        m_Computer->wake();
    }

private:
    NvComputer* m_Computer;
};

void ComputerModel::wakeComputer(int computerIndex)
{
    Q_ASSERT(computerIndex < m_Computers.count());

    DeferredWakeHostTask* wakeTask = new DeferredWakeHostTask(m_Computers[computerIndex]);
    QThreadPool::globalInstance()->start(wakeTask);
}

void ComputerModel::renameComputer(int computerIndex, QString name)
{
    Q_ASSERT(computerIndex < m_Computers.count());

    m_ComputerManager->renameHost(m_Computers[computerIndex], name);
}

QString ComputerModel::generatePinString()
{
    return m_ComputerManager->generatePinString();
}

class DeferredTestConnectionTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    void run()
    {
        unsigned int portTestResult = LiTestClientConnectivity("qt.conntest.moonlight-stream.org", 443, ML_PORT_FLAG_ALL);
        if (portTestResult == ML_TEST_RESULT_INCONCLUSIVE) {
            emit connectionTestCompleted(-1, QString());
        }
        else {
            char blockedPorts[512];
            LiStringifyPortFlags(portTestResult, "\n", blockedPorts, sizeof(blockedPorts));
            emit connectionTestCompleted(portTestResult, QString(blockedPorts));
        }
    }

signals:
    void connectionTestCompleted(int result, QString blockedPorts);
};

void ComputerModel::testConnectionForComputer(int)
{
    DeferredTestConnectionTask* testConnectionTask = new DeferredTestConnectionTask();
    QObject::connect(testConnectionTask, &DeferredTestConnectionTask::connectionTestCompleted,
                     this, &ComputerModel::connectionTestCompleted);
    QThreadPool::globalInstance()->start(testConnectionTask);
}

void ComputerModel::pairComputer(int computerIndex, QString pin)
{
    Q_ASSERT(computerIndex < m_Computers.count());

    m_ComputerManager->pairHost(m_Computers[computerIndex], pin);
}

void ComputerModel::handlePairingCompleted(NvComputer*, QString error)
{
    emit pairingCompleted(error.isEmpty() ? QVariant() : error);
}

void ComputerModel::handleComputerStateChanged(NvComputer* computer)
{
    QVector<NvComputer*> newComputerList = m_ComputerManager->getComputers();

    // Reset the model if the structural layout of the list has changed
    if (m_Computers != newComputerList) {
        beginResetModel();
        m_Computers = newComputerList;
        endResetModel();
    }
    else {
        // Let the view know that this specific computer changed
        int index = m_Computers.indexOf(computer);
        emit dataChanged(createIndex(index, 0), createIndex(index, 0));
    }
}

#include "computermodel.moc"
