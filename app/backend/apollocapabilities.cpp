#include "apollocapabilities.h"

#include <QRegularExpression>
#include <QXmlStreamReader>

namespace {
using State = ApolloCapabilities::State;

State booleanField(const QStringList& values, bool authenticated, bool& value)
{
    if (values.isEmpty()) {
        return State::Absent;
    }
    if (values.size() != 1 || (values.first() != "true" && values.first() != "false" &&
                              values.first() != "1" && values.first() != "0")) {
        return State::Invalid;
    }
    if (!authenticated) {
        return State::Unauthenticated;
    }
    value = values.first() == "true" || values.first() == "1";
    return State::Available;
}
}

ApolloCapabilities ApolloCapabilities::parse(const QString& xml, bool pinnedHttps)
{
    ApolloCapabilities result;
    const auto invalid = [&result]() {
        result.authenticated = false;
        result.permissionMask = 0;
        result.commandNames.clear();
        result.permissionsState = result.commandsState = result.displayCapableState = result.displayDriverState = State::Invalid;
        return result;
    };
    if (xml.size() > 1024 * 1024) {
        return invalid();
    }
    QXmlStreamReader reader(xml);
    QStringList permissions;
    QStringList commands;
    QStringList capable;
    QStringList ready;
    QStringList paired;
    bool rootFound = false;
    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        if (token == QXmlStreamReader::DTD || token == QXmlStreamReader::EntityReference) {
            return invalid();
        }
        if (token != QXmlStreamReader::StartElement) {
            continue;
        }
        if (rootFound || reader.qualifiedName().toString() != "root" || !reader.namespaceUri().isEmpty() ||
                reader.attributes().value("status_code").toString() != "200") {
            return invalid();
        }
        rootFound = true;
        while (reader.readNextStartElement()) {
            const QString name = reader.qualifiedName().toString();
            QStringList* field = nullptr;
            if (reader.namespaceUri().isEmpty()) {
                if (name == "Permission") field = &permissions;
                else if (name == "ServerCommand") field = &commands;
                else if (name == "VirtualDisplayCapable") field = &capable;
                else if (name == "VirtualDisplayDriverReady") field = &ready;
                else if (name == "PairStatus") field = &paired;
            }
            if (field) {
                const QString text = reader.readElementText(QXmlStreamReader::ErrorOnUnexpectedElement);
                field->append(name == "ServerCommand" ? text : text.trimmed());
                if (name != "PairStatus") {
                    result.advertised = true;
                }
                if (field->size() > 256) {
                    return invalid();
                }
            }
            else {
                reader.skipCurrentElement();
            }
        }
    }
    if (!rootFound || reader.hasError() || paired.size() > 1) {
        return invalid();
    }
    result.authenticated = pinnedHttps && paired == QStringList("1");
    if (!permissions.isEmpty()) {
        static const QRegularExpression decimal("^[0-9]{1,10}$");
        bool valid;
        const qulonglong mask = permissions.first().toULongLong(&valid);
        if (permissions.size() != 1 || !valid || !decimal.match(permissions.first()).hasMatch() || mask > 0xffffffffULL) {
            result.permissionsState = State::Invalid;
        }
        else if (!result.authenticated) {
            result.permissionsState = State::Unauthenticated;
        }
        else {
            result.permissionMask = static_cast<quint32>(mask);
            result.permissionsState = mask == 0 ? State::Denied : State::Available;
        }
    }
    result.displayCapableState = booleanField(capable, result.authenticated, result.displayCapable);
    result.displayDriverState = booleanField(ready, result.authenticated, result.displayDriverReady);
    const State commandPermission = result.permission(ServerCommand);
    if (commandPermission == State::Denied || commandPermission == State::Invalid || commandPermission == State::Unauthenticated) {
        result.commandsState = commandPermission;
    }
    else if (!commands.isEmpty()) {
        static const QRegularExpression controls("[\\x00-\\x1f\\x7f]");
        bool valid = true;
        for (const QString& name : commands) {
            if (name.trimmed().isEmpty() || name.size() > 512 || controls.match(name).hasMatch()) {
                valid = false;
                break;
            }
        }
        if (!valid) {
            result.commandsState = State::Invalid;
        }
        else if (!result.authenticated || commandPermission != State::Available) {
            result.commandsState = State::Unauthenticated;
        }
        else {
            result.commandsState = State::Available;
            result.commandNames = commands;
        }
    }
    return result;
}

bool ApolloCapabilities::acceptsPinnedReply(const QUrl& requestedOrigin, const QUrl& replyUrl,
                                          const QByteArray& pinnedCertificate, const QByteArray& peerCertificate)
{
    return requestedOrigin.isValid() && replyUrl.isValid() && requestedOrigin.scheme() == "https" &&
            replyUrl.scheme() == "https" && !requestedOrigin.host().isEmpty() && requestedOrigin.userInfo().isEmpty() &&
            requestedOrigin.port(443) > 0 && replyUrl.port(443) > 0 &&
            requestedOrigin.host() == replyUrl.host() && requestedOrigin.port(443) == replyUrl.port(443) &&
            replyUrl.path() == "/serverinfo" && replyUrl.userInfo().isEmpty() && !replyUrl.hasFragment() &&
            !pinnedCertificate.isEmpty() && pinnedCertificate == peerCertificate;
}

ApolloCapabilities::State ApolloCapabilities::permission(quint32 required) const
{
    if (required == 0) {
        return State::Invalid;
    }
    if (permissionsState != State::Available && permissionsState != State::Denied) {
        return permissionsState;
    }
    return (permissionMask & required) == required ? State::Available : State::Denied;
}

ApolloCapabilities ApolloCapabilities::withoutAuthentication() const
{
    auto result = *this;
    result.authenticated = false;
    result.permissionMask = 0;
    result.commandNames.clear();
    result.displayCapable = false;
    result.displayDriverReady = false;
    for (State* state : {&result.permissionsState, &result.commandsState, &result.displayCapableState, &result.displayDriverState}) {
        if (*state == State::Available || *state == State::Denied) {
            *state = State::Unauthenticated;
        }
    }
    return result;
}

QString ApolloCapabilities::stateName(State state)
{
    switch (state) {
    case State::Absent: return "absent";
    case State::Invalid: return "invalid";
    case State::Unauthenticated: return "unauthenticated";
    case State::Available: return "available";
    case State::Denied: return "denied";
    }
    return "invalid";
}

QVariantMap ApolloCapabilities::toVariantMap() const
{
    return {{"advertised", advertised}, {"authenticated", authenticated},
            {"permissionsState", stateName(permissionsState)}, {"permissionMask", permissionMask},
            {"commandsState", stateName(commandsState)}, {"commandNames", commandNames},
            {"displayCapableState", stateName(displayCapableState)}, {"displayCapable", displayCapable},
            {"displayDriverState", stateName(displayDriverState)}, {"displayDriverReady", displayDriverReady}};
}

bool ApolloCapabilities::operator==(const ApolloCapabilities& other) const
{
    return advertised == other.advertised && authenticated == other.authenticated &&
            permissionsState == other.permissionsState && permissionMask == other.permissionMask &&
            commandsState == other.commandsState && commandNames == other.commandNames &&
            displayCapableState == other.displayCapableState && displayCapable == other.displayCapable &&
            displayDriverState == other.displayDriverState && displayDriverReady == other.displayDriverReady;
}
