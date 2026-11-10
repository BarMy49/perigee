#include <QtTest>
#include <QDir>
#include <QFile>
#include <QXmlStreamReader>
#include <QTranslator>

class TranslationsTest : public QObject
{
    Q_OBJECT

private slots:
    void compiledCatalogs_data()
    {
        QTest::addColumn<QString>("catalogPath");
        const QDir directory(QFINDTESTDATA("../../app/languages"));
        const auto paths = directory.entryList({"qml_*.ts"}, QDir::Files);
        QCOMPARE(paths.size(), 30);
        for (const auto& path : paths) {
            QTest::newRow(qPrintable(path)) << directory.filePath(path);
        }
    }

    void compiledCatalogs()
    {
        QFETCH(QString, catalogPath);
        QTranslator translator;
        QVERIFY(translator.load(catalogPath.left(catalogPath.size() - 3) + ".qm"));
        QFile file(catalogPath);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QXmlStreamReader reader(&file);
        QString context;
        int checked = 0;
        while (!reader.atEnd()) {
            reader.readNext();
            if (!reader.isStartElement()) continue;
            if (reader.name() == QStringLiteral("name")) context = reader.readElementText();
            if (reader.name() != QStringLiteral("message")) continue;
            if (reader.attributes().value("numerus") == QStringLiteral("yes")) {
                reader.skipCurrentElement();
                continue;
            }
            QString source, comment, expected;
            bool active = true;
            while (reader.readNextStartElement()) {
                if (reader.name() == QStringLiteral("source")) source = reader.readElementText();
                else if (reader.name() == QStringLiteral("comment")) comment = reader.readElementText();
                else if (reader.name() == QStringLiteral("translation")) {
                    const auto type = reader.attributes().value("type");
                    active = type != QStringLiteral("unfinished") && type != QStringLiteral("vanished") && type != QStringLiteral("obsolete");
                    expected = reader.readElementText(QXmlStreamReader::IncludeChildElements);
                }
                else reader.skipCurrentElement();
            }
            if (!active || expected.isEmpty()) continue;
            const auto contextBytes = context.toUtf8();
            const auto sourceBytes = source.toUtf8();
            const auto commentBytes = comment.toUtf8();
            QCOMPARE(translator.translate(contextBytes.constData(), sourceBytes.constData(),
                    comment.isEmpty() ? nullptr : commentBytes.constData()), expected);
            checked++;
        }
        QVERIFY(!reader.hasError());
        QVERIFY(checked > 0);
        qInfo() << "Compiled translated lookups checked:" << checked;
    }
};

QTEST_GUILESS_MAIN(TranslationsTest)
#include "tst_translations.moc"
