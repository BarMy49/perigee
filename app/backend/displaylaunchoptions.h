#pragma once

#include "apollocapabilities.h"
#include <stdexcept>

class DisplayLaunchError : public std::runtime_error
{
public:
    explicit DisplayLaunchError(const QString& message) : std::runtime_error(message.toUtf8().constData()) {}
};

class DisplayLaunchOptions
{
public:
    enum class State { HostDefault, Ready, Unsupported, Unauthenticated, PermissionDenied, DriverUnavailable, NeedsFreshLaunch };
    static State evaluate(bool requested, bool isGfe, bool freshLaunch, const ApolloCapabilities& capabilities);
    static QString description(State state);
    static QString query(State state);
};
