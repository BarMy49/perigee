#include <QtTest>
#include "streaming/input/shortcutmodifiers.h"
#include "settings/localshortcuts.h"

class ShortcutModifiersTest : public QObject
{
    Q_OBJECT

private slots:
    void bindingPolicy()
    {
        QStringList bindings;
        QVERIFY(LocalShortcuts::resolve({}, bindings));
        QCOMPARE(bindings, LocalShortcuts::defaults());
        QVERIFY(LocalShortcuts::resolve({{"stats", "t"}, {"mouse", ""}}, bindings));
        QCOMPARE(bindings[3], QString("t"));
        QVERIFY(bindings[4].isEmpty());
        QCOMPARE(bindings[0], QString("q"));
        QCOMPARE(bindings[1], QString("z"));
        QCOMPARE(bindings[11], QString("e"));
        const QList<QMap<QString, QString>> invalid = {
            {{"stats", "z"}}, {{"stats", "q"}}, {{"stats", "e"}},
            {{"stats", "o"}}, {{"controls", "a"}}, {{"stats", "x"}}, {{"stats", "t"}, {"mouse", "t"}},
            {{"stats", "T"}}, {{"stats", "ctrl+t"}}, {{"stats", "ą"}},
            {{"unknown", "t"}}, {{"ungrab", ""}}, {{"quit", "t"}},
            {{"quitAndExit", ""}}, {{"clipboardReceive", "k"}}
        };
        for (const auto& overrides : invalid) {
            QVERIFY(!LocalShortcuts::resolve(overrides, bindings));
            QCOMPARE(bindings, LocalShortcuts::defaults());
        }
        QVERIFY(LocalShortcuts::resolve({{"keyboardGrab", "b"}}, bindings));
        QCOMPARE(bindings[9], QString("k"));
        QCOMPARE(bindings[12], QString("b"));
    }

    void persistedBindings()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("settings.ini");
        {
            QSettings settings(path, QSettings::IniFormat);
            settings.setValue("localshortcuts/stats", "t");
            settings.setValue("localshortcuts/mouse", "");
            settings.sync();
            QCOMPARE(settings.status(), QSettings::NoError);
        }
        QSettings settings(path, QSettings::IniFormat);
        QStringList bindings;
        QVERIFY(LocalShortcuts::load(settings, bindings));
        QCOMPARE(bindings[3], QString("t"));
        QVERIFY(bindings[4].isEmpty());
        settings.setValue("localshortcuts/stats", "z");
        QVERIFY(!LocalShortcuts::load(settings, bindings));
        QCOMPARE(bindings, LocalShortcuts::defaults());
        QCOMPARE(settings.group(), QString());
        QCOMPARE(settings.value("localshortcuts/stats").toString(), QString("z"));
    }

    void consumedReleases()
    {
        LocalShortcut::ConsumedKeys keys;
        SDL_KeyboardEvent event{};
        event.keysym.scancode = SDL_SCANCODE_T;
        event.state = SDL_PRESSED;
        QVERIFY(!keys.filter(event));
        keys.consume(SDL_SCANCODE_T);
        event.repeat = 1;
        QVERIFY(keys.filter(event));
        event.repeat = 0;
        event.state = SDL_RELEASED;
        QVERIFY(keys.filter(event));
        QVERIFY(!keys.filter(event));
        keys.consume(SDL_SCANCODE_T);
        event.state = SDL_PRESSED;
        QVERIFY(!keys.filter(event));
        event.state = SDL_RELEASED;
        QVERIFY(!keys.filter(event));
        event.keysym.scancode = static_cast<SDL_Scancode>(SDL_NUM_SCANCODES);
        QVERIFY(!keys.filter(event));
    }

    void modifierCombinations()
    {
        const Uint16 modifierBits[] = {
            KMOD_LCTRL, KMOD_RCTRL, KMOD_LALT, KMOD_RALT,
            KMOD_LSHIFT, KMOD_RSHIFT, KMOD_LGUI, KMOD_RGUI,
            KMOD_NUM, KMOD_CAPS, KMOD_MODE
        };
        for (unsigned combination = 0; combination < (1u << 11); combination++) {
            Uint16 modifiers = 0;
            for (unsigned bit = 0; bit < 11; bit++) {
                if (combination & (1u << bit)) {
                    modifiers |= modifierBits[bit];
                }
            }
            const bool hasControl = combination & 3u;
            const bool hasAlt = combination & 12u;
            const bool hasShift = combination & 48u;
            const bool hasAltGr = combination & 1024u;
            QCOMPARE(LocalShortcut::modifiersMatch(modifiers),
                     hasControl && hasAlt && hasShift && !hasAltGr);
        }
    }
};

QTEST_APPLESS_MAIN(ShortcutModifiersTest)
#include "tst_shortcutmodifiers.moc"
