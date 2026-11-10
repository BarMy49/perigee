#pragma once

#include "apollocapabilities.h"
#include <array>
#include <optional>

namespace ServerCommandRequest
{
inline std::optional<std::array<quint8, 4>> prepare(const ApolloCapabilities& capabilities,
        const QStringList& displayedCommands, quint64 displayedGeneration, quint64 activeGeneration,
        bool active, int index)
{
    if (!active || displayedGeneration == 0 || displayedGeneration != activeGeneration ||
            !capabilities.authenticated || !capabilities.advertised ||
            capabilities.permission(ApolloCapabilities::ServerCommand) != ApolloCapabilities::State::Available ||
            capabilities.commandsState != ApolloCapabilities::State::Available ||
            capabilities.commandNames != displayedCommands || displayedCommands.size() > 256 ||
            index < 0 || index > 255 || index >= displayedCommands.size() || displayedCommands[index].isEmpty()) return {};
    return std::array<quint8, 4>{static_cast<quint8>(index), 0, 0, 0};
}
}
