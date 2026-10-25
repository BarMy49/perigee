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
