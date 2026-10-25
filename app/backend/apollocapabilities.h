#pragma once

#include <QStringList>
#include <QUrl>
#include <QVariantMap>

class ApolloCapabilities
{
public:
    enum class State { Absent, Invalid, Unauthenticated, Available, Denied };
    enum Permission : quint32 {
        Controller = 0x00000100,
        Touch = 0x00000200,
        Pen = 0x00000400,
        Mouse = 0x00000800,
        Keyboard = 0x00001000,
        ClipboardSet = 0x00010000,
        ClipboardRead = 0x00020000,
        ServerCommand = 0x00100000,
        List = 0x01000000,
        View = 0x02000000,
        Launch = 0x04000000
    };

    static ApolloCapabilities parse(const QString& xml, bool pinnedHttps);
    static bool acceptsPinnedReply(const QUrl& requestedOrigin, const QUrl& replyUrl,
                                   const QByteArray& pinnedCertificate, const QByteArray& peerCertificate);
    static QString stateName(State state);
    State permission(quint32 required) const;
    ApolloCapabilities withoutAuthentication() const;
    QVariantMap toVariantMap() const;
    bool operator==(const ApolloCapabilities& other) const;
    bool operator!=(const ApolloCapabilities& other) const { return !(*this == other); }

    bool advertised = false;
    bool authenticated = false;
    State permissionsState = State::Absent;
    quint32 permissionMask = 0;
    State commandsState = State::Absent;
    QStringList commandNames;
    State displayCapableState = State::Absent;
    bool displayCapable = false;
    State displayDriverState = State::Absent;
    bool displayDriverReady = false;
};
