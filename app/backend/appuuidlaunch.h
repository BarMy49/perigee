#pragma once

#include "apollocapabilities.h"
#include <QRegularExpression>
#include <QUuid>

namespace AppUuidLaunch
{
inline QString normalized(const QString& text)
{
    static const QRegularExpression pattern("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");
    const QUuid uuid(text);
    return pattern.match(text).hasMatch() && !uuid.isNull() ? uuid.toString(QUuid::WithoutBraces) : QString();
}

inline QString query(int appId, const QString& uuid, bool ready)
{
    const QString identity = normalized(uuid);
    return ready && !identity.isEmpty() ? "appid=0&appuuid=" + identity : "appid=" + QString::number(appId);
}

template<class AppList>
inline bool matchesAppIdentity(const AppList& apps, int appId, const QString& uuid)
{
    const QString identity = normalized(uuid);
    if (appId <= 0 || identity.isEmpty()) return false;
    int numericMatches = 0;
    int uuidMatches = 0;
    bool matchingPair = false;
    for (const auto& app : apps) {
        if (app.id == appId) numericMatches++;
        if (app.uuid == identity) uuidMatches++;
        if (app.id == appId && app.uuid == identity) matchingPair = true;
    }
    return matchingPair && numericMatches == 1 && uuidMatches == 1;
}

inline bool acceptsAppListReply(const QUrl& origin, const QUrl& reply,
                                const QByteArray& pin, const QByteArray& peer)
{
    return origin.isValid() && reply.isValid() && origin.scheme() == "https" && reply.scheme() == "https" &&
            !origin.host().isEmpty() && origin.host() == reply.host() && origin.port(443) > 0 &&
            origin.port(443) == reply.port(443) && origin.userInfo().isEmpty() && reply.userInfo().isEmpty() &&
            reply.path() == "/applist" && !reply.hasFragment() && !pin.isEmpty() && pin == peer;
}

inline bool eligible(bool isGfe, bool freshLaunch, const ApolloCapabilities& capabilities,
                     bool pinnedAppList, bool matchingApp, const QString& uuid)
{
    return !isGfe && freshLaunch && capabilities.advertised && capabilities.authenticated &&
            capabilities.permission(ApolloCapabilities::Launch) == ApolloCapabilities::State::Available &&
            pinnedAppList && matchingApp && !normalized(uuid).isEmpty();
}
}
