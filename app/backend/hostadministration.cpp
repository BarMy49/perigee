#include "hostadministration.h"

#include <QRegularExpression>

QUrl HostAdministration::validatedUrl(const QString& text)
{
    static const QRegularExpression controls("[\\x00-\\x20\\x7f]");
    const QString trimmed = text.trimmed();
    const QUrl url(trimmed, QUrl::StrictMode);
    if (trimmed.isEmpty() || trimmed.size() > 2048 || controls.match(trimmed).hasMatch() ||
            !url.isValid() || url.scheme() != "https" || url.host().isEmpty() ||
            url.authority().contains('@') || url.hasQuery() || url.hasFragment() ||
            url.port(443) < 1 || url.port(443) > 65535 || controls.match(url.path(QUrl::FullyDecoded)).hasMatch()) {
        return {};
    }
    return url;
}

QUrl HostAdministration::validatedSftpUrl(const QString& text)
{
    const QString trimmed = text.trimmed();
    const QUrl url(trimmed, QUrl::StrictMode);
    const QRegularExpression controls("[\\x00-\\x20\\x7f]");
    const QRegularExpression decodedControls("[\\x00-\\x1f\\x7f]");
    if (trimmed.isEmpty() || trimmed.size() > 2048 || controls.match(trimmed).hasMatch() ||
            !url.isValid() || url.scheme() != "sftp" || url.host().isEmpty() ||
            url.authority().contains('@') || url.hasQuery() || url.hasFragment() ||
            url.port(22) < 1 || url.port(22) > 65535 ||
            decodedControls.match(url.path(QUrl::FullyDecoded)).hasMatch()) return {};
    return url;
}

bool HostAdministration::openSftpUrl(const QString& text, const std::function<bool(const QUrl&)>& opener)
{
    const QUrl url = validatedSftpUrl(text);
    return !url.isEmpty() && opener(url);
}
