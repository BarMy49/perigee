#pragma once

#include <QByteArray>
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#else
#include <QTextCodec>
#endif

namespace ClipboardText {
inline bool valid(const QByteArray& text)
{
    if (text.size() > 1024 * 1024 || text.contains('\0')) return false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder decoder(QStringDecoder::Utf8, QStringConverter::Flag::Stateless);
    const QString decoded = decoder.decode(text);
    Q_UNUSED(decoded);
    return !decoder.hasError();
#else
    QTextCodec::ConverterState state;
    QTextCodec::codecForName("UTF-8")->toUnicode(text.constData(), text.size(), &state);
    return state.invalidChars == 0 && state.remainingChars == 0;
#endif
}
}
