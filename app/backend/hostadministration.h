#pragma once

#include <QUrl>
#include <functional>

class HostAdministration
{
public:
    static bool openSftpUrl(const QString& text, const std::function<bool(const QUrl&)>& opener);
    static QUrl validatedSftpUrl(const QString& text);
    static QUrl validatedUrl(const QString& text);
};
