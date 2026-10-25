#include <QtTest>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QTemporaryDir>

#include "settings/moonlightimporter.h"
#include "settings/profilestore.h"

#include <openssl/pem.h>
#include <memory>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

class IdentityImportTest : public QObject
{
    Q_OBJECT

private:
    QByteArray m_Certificate;
    QByteArray m_Key;
    QByteArray m_OtherKey;
    QByteArray m_EncryptedKey;

    static QByteArray read(const QString& name)
    {
        QFile file(name);
        if (!file.open(QIODevice::ReadOnly)) {
            return {};
        }
        return file.readAll();
    }

    void fixture(const QString& name)
    {
        QSettings settings(name, QSettings::IniFormat);
        settings.setValue("width", 2560);
        settings.setValue("height", 1440);
        settings.setValue("fps", 120);
        settings.setValue("vsync", true);
        settings.setValue("unknown/futureSetting", "excluded");
        settings.setValue("renderer", 3);
        settings.setValue("certificate", m_Certificate);
        settings.setValue("key", m_Key);
        settings.setValue("uniqueid", "00Ab0123456789Cd");
        settings.setValue("hosts/size", 1);
        host(settings, "hosts/1/");
        settings.sync();
    }

    void host(QSettings& settings, const QString& prefix)
    {
        settings.setValue(prefix + "hostname", QString::fromUtf8("Test \xc5\x81\xc3\xb3\xc4\x85 host"));
        settings.setValue(prefix + "uuid", "12345678-1234-5678-90ab-1234567890ab");
        settings.setValue(prefix + "localaddress", "192.0.2.10");
        settings.setValue(prefix + "localport", 47989);
        settings.setValue(prefix + "mac", QByteArray::fromHex("010203040506"));
        settings.setValue(prefix + "srvcert", m_Certificate);
        settings.setValue(prefix + "apps/size", 1);
        settings.setValue(prefix + "apps/1/name", "Desktop");
        settings.setValue(prefix + "apps/1/id", 1);
        settings.setValue(prefix + "apps/1/hidden", true);
    }

    static QProcessEnvironment environment(const QString& root)
    {
        auto result = QProcessEnvironment::systemEnvironment();
        for (const auto& key : {QString("XDG_CONFIG_HOME"), QString("XDG_CACHE_HOME"), QString("XDG_DATA_HOME"), QString("TMPDIR")}) {
            const QString path = root + "/" + key;
            QDir().mkpath(path);
            result.insert(key, path);
        }
        result.insert("QT_QPA_PLATFORM", "not-a-platform");
        result.insert("DISPLAY", "");
        result.insert("WAYLAND_DISPLAY", "");
        result.remove("QML_DISK_CACHE_PATH");
        return result;
    }

private slots:
    void initTestCase()
    {
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key(EVP_RSA_gen(2048), EVP_PKEY_free);
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> other(EVP_RSA_gen(2048), EVP_PKEY_free);
        std::unique_ptr<X509, decltype(&X509_free)> certificate(X509_new(), X509_free);
        QVERIFY(key && other && certificate);
        QVERIFY(X509_set_version(certificate.get(), 2) == 1);
        QVERIFY(ASN1_INTEGER_set(X509_get_serialNumber(certificate.get()), 1) == 1);
        QVERIFY(X509_gmtime_adj(X509_getm_notBefore(certificate.get()), 0));
        QVERIFY(X509_gmtime_adj(X509_getm_notAfter(certificate.get()), 86400));
        QVERIFY(X509_set_pubkey(certificate.get(), key.get()) == 1);
        auto subject = X509_get_subject_name(certificate.get());
        QVERIFY(X509_NAME_add_entry_by_txt(subject, "CN", MBSTRING_ASC,
                                         reinterpret_cast<const unsigned char*>("Synthetic import fixture"), -1, -1, 0) == 1);
        QVERIFY(X509_set_issuer_name(certificate.get(), subject) == 1);
        QVERIFY(X509_sign(certificate.get(), key.get(), EVP_sha256()) > 0);
        std::unique_ptr<BIO, decltype(&BIO_free)> buffer(BIO_new(BIO_s_mem()), BIO_free);
        QVERIFY(buffer);
        QVERIFY(PEM_write_bio_X509(buffer.get(), certificate.get()) == 1);
        BUF_MEM* bytes;
        BIO_get_mem_ptr(buffer.get(), &bytes);
        m_Certificate = QByteArray(bytes->data, bytes->length);
        QVERIFY(BIO_reset(buffer.get()) == 1);
        QVERIFY(PEM_write_bio_PrivateKey(buffer.get(), key.get(), nullptr, nullptr, 0, nullptr, nullptr) == 1);
        BIO_get_mem_ptr(buffer.get(), &bytes);
        m_Key = QByteArray(bytes->data, bytes->length);
        QVERIFY(BIO_reset(buffer.get()) == 1);
        QVERIFY(PEM_write_bio_PrivateKey(buffer.get(), other.get(), nullptr, nullptr, 0, nullptr, nullptr) == 1);
        BIO_get_mem_ptr(buffer.get(), &bytes);
        m_OtherKey = QByteArray(bytes->data, bytes->length);
        QVERIFY(BIO_reset(buffer.get()) == 1);
        const QByteArray password("synthetic-password");
        QVERIFY(PEM_write_bio_PrivateKey(buffer.get(), key.get(), EVP_aes_256_cbc(),
                                        reinterpret_cast<const unsigned char*>(password.constData()), password.size(), nullptr, nullptr) == 1);
        BIO_get_mem_ptr(buffer.get(), &bytes);
        m_EncryptedKey = QByteArray(bytes->data, bytes->length);
    }

    void explicitIdentity_data()
    {
        QTest::addColumn<bool>("includePairing");
        QTest::addColumn<bool>("crlf");
        QTest::newRow("preferences-and-hosts") << false << false;
        QTest::newRow("explicit-identity") << true << false;
        QTest::newRow("explicit-identity-crlf") << true << true;
    }

    void explicitIdentity()
    {
        QFETCH(bool, includePairing);
        QFETCH(bool, crlf);
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QString source = root.filePath("Moonlight.conf");
        const QString destination = root.filePath("profile/Perigee.conf");
        fixture(source);
        const QByteArray expectedCertificate = crlf ? QByteArray(m_Certificate).replace("\n", "\r\n") : m_Certificate;
        const QByteArray expectedKey = crlf ? QByteArray(m_Key).replace("\n", "\r\n") : m_Key;
        if (crlf) {
            QSettings settings(source, QSettings::IniFormat);
            settings.setValue("certificate", expectedCertificate);
            settings.setValue("key", expectedKey);
        }
        const auto original = read(source);
        QVERIFY(QFile::setPermissions(source, QFile::ReadOwner));
        const auto result = MoonlightImporter::importSettings(source, destination, includePairing);
        QVERIFY2(result.success, qPrintable(result.error));
        QCOMPARE(result.preferenceCount, 4);
        QCOMPARE(result.hostCount, 1);
        QVERIFY(read(source) == original);
        QSettings settings(destination, QSettings::IniFormat);
        QCOMPARE(settings.value("width").toInt(), 2560);
        QCOMPARE(settings.value("hosts/1/apps/1/name").toString(), QString("Desktop"));
        QVERIFY(settings.value("hosts/1/apps/1/hidden").toBool());
        QVERIFY(settings.value("hosts/1/srvcert").toByteArray() == m_Certificate);
        QVERIFY(!settings.contains("unknown/futureSetting"));
        QVERIFY(!settings.contains("renderer"));
        QCOMPARE(settings.contains("certificate"), includePairing);
        QCOMPARE(settings.contains("key"), includePairing);
        QCOMPARE(settings.contains("uniqueid"), includePairing);
        if (includePairing) {
            QVERIFY(settings.value("certificate").toByteArray() == expectedCertificate);
            QVERIFY(settings.value("key").toByteArray() == expectedKey);
            QVERIFY(settings.value("uniqueid").toString() == "00Ab0123456789Cd");
        }
        QCOMPARE(QFile::permissions(destination) & (QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther | QFile::WriteOther), QFile::Permissions());
        const auto published = read(destination);
        QVERIFY(!MoonlightImporter::importSettings(source, destination, includePairing).success);
        QVERIFY(read(destination) == published);
    }

    void backupPrecedence()
    {
        QTemporaryDir root;
        const QString source = root.filePath("Moonlight.ini");
        fixture(source);
        {
            QSettings settings(source, QSettings::IniFormat);
            settings.setValue("hosts/1/uuid", "broken-primary");
            settings.setValue("hostsbackup/size", 1);
            host(settings, "hostsbackup/1/");
            settings.setValue("hostsbackup/1/hostname", "Recovered host");
            settings.remove("hostsbackup/1/apps");
        }
        const auto result = MoonlightImporter::importSettings(source, root.filePath("Perigee.ini"), false);
        QVERIFY2(result.success, qPrintable(result.error));
        QVERIFY(result.usedHostBackup);
        QSettings settings(root.filePath("Perigee.ini"), QSettings::IniFormat);
        QCOMPARE(settings.value("hosts/1/hostname").toString(), QString("Recovered host"));
        QCOMPARE(settings.value("hosts/1/apps/size").toInt(), 0);
        QVERIFY(!settings.contains("hostsbackup/size"));
    }

    void rejectedValues_data()
    {
        QTest::addColumn<QString>("key");
        QTest::addColumn<QVariant>("value");
        QTest::addColumn<bool>("pairing");
        QTest::newRow("bad-boolean") << QString("vsync") << QVariant("yes") << false;
        QTest::newRow("bad-resolution") << QString("width") << QVariant(-1) << false;
        QTest::newRow("bad-fps") << QString("fps") << QVariant(10000) << false;
        QTest::newRow("bad-language") << QString("language") << QVariant(32) << false;
        QTest::newRow("bad-packet-size") << QString("packetsize") << QVariant(1) << false;
        QTest::newRow("bad-host-count") << QString("hosts/size") << QVariant(-1) << false;
        QTest::newRow("bad-backup-count") << QString("hostsbackup/size") << QVariant("x") << false;
        QTest::newRow("bad-uuid") << QString("hosts/1/uuid") << QVariant("not-uuid") << false;
        QTest::newRow("bad-pin") << QString("hosts/1/srvcert") << QVariant(QByteArray("invalid PEM")) << false;
        QTest::newRow("bad-port") << QString("hosts/1/localport") << QVariant(65536) << false;
        QTest::newRow("bad-address") << QString("hosts/1/localaddress") << QVariant("https://invalid/") << false;
        QTest::newRow("bad-app-count") << QString("hosts/1/apps/size") << QVariant(20000) << false;
        QTest::newRow("bad-app-id") << QString("hosts/1/apps/1/id") << QVariant(-1) << false;
        QTest::newRow("bad-app-flag") << QString("hosts/1/apps/1/hidden") << QVariant("yes") << false;
        QTest::newRow("bad-client-id") << QString("uniqueid") << QVariant("not-hex") << true;
        QTest::newRow("bad-private-key") << QString("key") << QVariant(QByteArray("invalid PEM")) << true;
        QTest::newRow("mismatched-private-key") << QString("key") << QVariant(m_OtherKey) << true;
        QTest::newRow("encrypted-private-key") << QString("key") << QVariant(m_EncryptedKey) << true;
        QTest::newRow("missing-certificate") << QString("certificate") << QVariant(QByteArray()) << true;
    }

    void rejectedValues()
    {
        QFETCH(QString, key);
        QFETCH(QVariant, value);
        QFETCH(bool, pairing);
        QTemporaryDir root;
        const QString source = root.filePath("source.ini");
        fixture(source);
        {
            QSettings settings(source, QSettings::IniFormat);
            settings.setValue(key, value);
        }
        const auto before = read(source);
        const QString destination = root.filePath("destination.ini");
        QVERIFY(!MoonlightImporter::importSettings(source, destination, pairing).success);
        QVERIFY(!QFileInfo::exists(destination));
        QVERIFY(read(source) == before);
        QVERIFY(QDir(root.path()).entryList({".perigee-import-*"}, QDir::Files | QDir::Hidden).isEmpty());
    }

    void rejectsMalformedFiles()
    {
        QTemporaryDir root;
        const QString source = root.filePath("broken.ini");
        const QList<QByteArray> cases = {QByteArray(), QByteArray("[General\nwidth=100\n"), QByteArray("\xff", 1),
                                       QByteArray("[General]\nwidth=100\n") + QByteArray(1, '\0'), QByteArray("[General]\nunknown=1\n")};
        for (const auto& bytes : cases) {
            QFile file(source);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QVERIFY(file.write(bytes) == bytes.size());
            file.close();
            QVERIFY(!MoonlightImporter::importSettings(source, root.filePath("destination.ini"), false).success);
            QVERIFY(!QFileInfo::exists(root.filePath("destination.ini")));
        }
    }

    void rejectsDuplicateHosts()
    {
        QTemporaryDir root;
        const QString source = root.filePath("source.ini");
        fixture(source);
        {
            QSettings settings(source, QSettings::IniFormat);
            settings.setValue("hosts/size", 2);
            host(settings, "hosts/2/");
        }
        QVERIFY(!MoonlightImporter::importSettings(source, root.filePath("destination.ini"), false).success);
    }

    void rejectsOversizedSource()
    {
        QTemporaryDir root;
        QFile source(root.filePath("oversized.ini"));
        QVERIFY(source.open(QIODevice::WriteOnly));
        QVERIFY(source.resize(8 * 1024 * 1024 + 1));
        source.close();
        const QString destination = root.filePath("destination.ini");
        QVERIFY(!MoonlightImporter::importSettings(source.fileName(), destination, false).success);
        QVERIFY(!QFileInfo::exists(destination));
        QCOMPARE(source.size(), qint64(8 * 1024 * 1024 + 1));
    }

    void refusesExistingAndBusyProfiles()
    {
        QTemporaryDir root;
        const QString source = root.filePath("source.ini");
        const QString destination = root.filePath("destination.ini");
        fixture(source);
        {
            ProfileLock lock(destination);
            QVERIFY(lock.tryLock());
            QVERIFY(!MoonlightImporter::importSettings(source, destination, true).success);
            QVERIFY(!QFileInfo::exists(destination));
        }
        QFile existing(destination);
        QVERIFY(existing.open(QIODevice::WriteOnly));
        existing.close();
        QVERIFY(!MoonlightImporter::importSettings(source, destination, true).success);
        QCOMPARE(QFileInfo(destination).size(), qint64(0));
        QVERIFY(!MoonlightImporter::importSettings(source, source, true).success);
        QVERIFY(existing.remove());
        QVERIFY(QFile::link(source, destination));
        QVERIFY(!MoonlightImporter::importSettings(source, destination, true).success);
        QVERIFY(QFileInfo(destination).isSymLink());
    }

    void atomicPublicationRace()
    {
        QTemporaryDir root;
        const QString destination = root.filePath("profile.ini");
        ProfilePublication publication;
        QVERIFY(publication.prepare(destination, "key=synthetic-private-fixture\n"));
        QFile raced(destination);
        QVERIFY(raced.open(QIODevice::WriteOnly));
        QVERIFY(raced.write("raced") == 5);
        raced.close();
        QVERIFY(!publication.commit());
        QVERIFY(read(destination) == "raced");
    }

    void interruptedPublication()
    {
        QTemporaryDir root;
        const QString destination = root.filePath("profile.ini");
        int ready[2];
        QVERIFY(::pipe(ready) == 0);
        const pid_t child = ::fork();
        QVERIFY(child >= 0);
        if (child == 0) {
            ::close(ready[0]);
            ProfileLock lock(destination);
            ProfilePublication publication;
            const bool prepared = lock.tryLock() && publication.prepare(destination, "key=synthetic-private-fixture\n");
            const char status = prepared ? '1' : '0';
            ::write(ready[1], &status, 1);
            ::close(ready[1]);
            ::pause();
            ::_exit(1);
        }
        ::close(ready[1]);
        char status = '0';
        const auto received = ::read(ready[0], &status, 1);
        ::close(ready[0]);
        ::kill(child, SIGKILL);
        int exitStatus;
        QVERIFY(::waitpid(child, &exitStatus, 0) == child);
        QVERIFY(received == 1 && status == '1');
        QVERIFY(WIFSIGNALED(exitStatus));
        QVERIFY(!QFileInfo::exists(destination));
        QVERIFY(QDir(root.path()).entryList({"*"}, QDir::Files | QDir::Hidden).size() == 1);
        ProfileLock recovered(destination);
        QVERIFY(recovered.tryLock());
    }

    void headlessCliAndConcurrentImports()
    {
        const QString binary = qEnvironmentVariable("PERIGEE_TEST_BINARY");
        QVERIFY2(QFileInfo(binary).isExecutable(), "Set PERIGEE_TEST_BINARY to the built Linux perigee executable.");
        QTemporaryDir root;
        const QString source = root.filePath("Moonlight.ini");
        fixture(source);
        const auto original = read(source);
        QProcess first;
        QProcess second;
        for (QProcess* process : {&first, &second}) {
            process->setProcessEnvironment(environment(root.path()));
            process->setWorkingDirectory(root.path());
            process->start(binary, {"import-moonlight", "--source", source, "--include-pairing"});
        }
        QVERIFY(first.waitForFinished(10000));
        QVERIFY(second.waitForFinished(10000));
        QVERIFY((first.exitCode() == 0 && second.exitCode() == 1) || (first.exitCode() == 1 && second.exitCode() == 0));
        const QByteArray messages = first.readAllStandardOutput() + first.readAllStandardError() + second.readAllStandardOutput() + second.readAllStandardError();
        QVERIFY(!messages.contains(m_Key));
        QVERIFY(!messages.contains(m_Certificate));
        QVERIFY(!messages.contains("00Ab0123456789Cd"));
        QVERIFY(messages.contains("share host-side identity"));
        QVERIFY(read(source) == original);
        QSettings imported(root.filePath("XDG_CONFIG_HOME/io.github.BarMy49/Perigee.conf"), QSettings::IniFormat);
        QVERIFY(imported.value("key").toByteArray() == m_Key);
        QProcess invalid;
        invalid.setProcessEnvironment(environment(root.filePath("invalid")));
        invalid.setWorkingDirectory(root.path());
        invalid.start(binary, {"import-moonlight", "--source", source, "--unexpected"});
        QVERIFY(invalid.waitForFinished(10000));
        QCOMPARE(invalid.exitCode(), 2);
        QVERIFY(!QFileInfo::exists(root.filePath("invalid/XDG_CONFIG_HOME/io.github.BarMy49/Perigee.conf")));
        QFile marker(root.filePath("perigee-portable.dat"));
        QVERIFY(marker.open(QIODevice::WriteOnly));
        marker.close();
        QProcess portable;
        portable.setProcessEnvironment(environment(root.path()));
        portable.setWorkingDirectory(root.path());
        portable.start(binary, {"import-moonlight", "--source", source});
        QVERIFY(portable.waitForFinished(10000));
        QCOMPARE(portable.exitCode(), 0);
        QSettings portableSettings(root.filePath("perigee-data/config/io.github.BarMy49/Perigee.ini"), QSettings::IniFormat);
        QVERIFY(!portableSettings.contains("key"));
        QCOMPARE(portableSettings.value("width").toInt(), 2560);
    }
};

QTEST_GUILESS_MAIN(IdentityImportTest)
#include "tst_identityimport.moc"
