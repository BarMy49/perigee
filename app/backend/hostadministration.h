#pragma once

#include <QUrl>

class HostAdministration
{
public:
    static QUrl validatedUrl(const QString& text);
};
