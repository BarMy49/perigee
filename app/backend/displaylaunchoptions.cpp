#include "displaylaunchoptions.h"
#include <QCoreApplication>

DisplayLaunchOptions::State DisplayLaunchOptions::evaluate(bool requested, bool isGfe, bool freshLaunch, const ApolloCapabilities& capabilities)
{
    using Availability = ApolloCapabilities::State;
    if (!requested) return State::HostDefault;
    if (isGfe || !capabilities.advertised) return State::Unsupported;
    if (!capabilities.authenticated) return State::Unauthenticated;
    if (capabilities.permission(ApolloCapabilities::Launch) != Availability::Available) return State::PermissionDenied;
    if (capabilities.displayCapableState != Availability::Available || !capabilities.displayCapable) return State::Unsupported;
    if (capabilities.displayDriverState != Availability::Available || !capabilities.displayDriverReady) return State::DriverUnavailable;
    if (!freshLaunch) return State::NeedsFreshLaunch;
    return State::Ready;
}

QString DisplayLaunchOptions::description(State state)
{
    switch (state) {
    case State::HostDefault:
        return QCoreApplication::translate("DisplayLaunchOptions", "Use host display defaults.");
    case State::Ready:
        return QCoreApplication::translate("DisplayLaunchOptions", "Host reports virtual-display support, a ready driver and launch permission.");
    case State::Unsupported:
        return QCoreApplication::translate("DisplayLaunchOptions", "This host does not advertise supported virtual-display launch controls.");
    case State::Unauthenticated:
        return QCoreApplication::translate("DisplayLaunchOptions", "Current paired HTTPS capability information is required.");
    case State::PermissionDenied:
        return QCoreApplication::translate("DisplayLaunchOptions", "Host launch permission is unavailable. Check this client's permissions on the host.");
    case State::DriverUnavailable:
        return QCoreApplication::translate("DisplayLaunchOptions", "The host virtual-display driver is not reported ready. Check the driver on the host.");
    case State::NeedsFreshLaunch:
        return QCoreApplication::translate("DisplayLaunchOptions", "A virtual display can only be requested for a new launch. Disable this option to resume, or end the host application yourself first.");
    }
    return {};
}

QString DisplayLaunchOptions::query(State state)
{
    return state == State::Ready ? QString("&virtualDisplay=1") : QString();
}
