#pragma once

#include <QMap>
#include <QStringList>
#include <QSettings>
#include <QCoreApplication>

namespace LocalShortcuts
{
inline QStringList actionNames()
{
    return {"quit", "ungrab", "fullscreen", "stats", "mouse", "cursor",
            "minimize", "paste", "clipboardSend", "clipboardReceive",
            "pointerLock", "quitAndExit", "keyboardGrab", "controls"};
}

inline QStringList defaults()
{
    return {"q", "z", "x", "s", "m", "c", "d", "v", "j", "k", "l", "e", "k", "o"};
}

inline QStringList labels()
{
    return {
        QCoreApplication::translate("StreamControls", "Disconnect"),
        QCoreApplication::translate("StreamControls", "Release capture"),
        QCoreApplication::translate("StreamControls", "Fullscreen"),
        QCoreApplication::translate("StreamControls", "Statistics"),
        QCoreApplication::translate("StreamControls", "Mouse mode"),
        QCoreApplication::translate("StreamControls", "Cursor visibility"),
        QCoreApplication::translate("StreamControls", "Minimize"),
        QCoreApplication::translate("StreamControls", "Type clipboard text"),
        QCoreApplication::translate("StreamControls", "Send clipboard"),
        QCoreApplication::translate("StreamControls", "Receive clipboard"),
        QCoreApplication::translate("StreamControls", "Pointer lock"),
        QCoreApplication::translate("StreamControls", "Disconnect and exit"),
        QCoreApplication::translate("StreamControls", "Keyboard capture"),
        QCoreApplication::translate("StreamControls", "Stream controls")
    };
}

inline bool resolve(const QMap<QString, QString>& overrides, QStringList& bindings)
{
    const QStringList names = actionNames();
    bindings = defaults();
    for (auto entry = overrides.cbegin(); entry != overrides.cend(); ++entry) {
        const int index = names.indexOf(entry.key());
        const QString value = entry.value();
        if (index < 0 || (index == 0 || index == 1 || index == 11 || index == 13) ||
                (!value.isEmpty() && (value.size() != 1 || value[0] < QLatin1Char('a') ||
                                     value[0] > QLatin1Char('z')))) {
            bindings = defaults();
            return false;
        }
        bindings[index] = value;
    }
    for (int first = 0; first < bindings.size(); first++) {
        for (int second = first + 1; second < bindings.size(); second++) {
            if (!bindings[first].isEmpty() && bindings[first] == bindings[second] &&
                    !(first == 9 && second == 12 && bindings[first] == QLatin1String("k") &&
                      !overrides.contains(names[first]) && !overrides.contains(names[second]))) {
                bindings = defaults();
                return false;
            }
        }
    }
    return true;
}

inline bool load(QSettings& settings, QStringList& bindings)
{
    QMap<QString, QString> overrides;
    settings.beginGroup("localshortcuts");
    for (const QString& action : settings.childKeys()) {
        overrides.insert(action, settings.value(action).toString());
    }
    settings.endGroup();
    return resolve(overrides, bindings);
}
}
