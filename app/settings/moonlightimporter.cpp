#include "moonlightimporter.h"
#include "profilestore.h"

#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QHostAddress>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QSslCertificate>
#include <QTemporaryFile>
#include <QTextStream>
#include <QUuid>

#include <openssl/pem.h>
#include <memory>

namespace {
constexpr qint64 MaximumSourceSize = 8 * 1024 * 1024;
constexpr int MaximumHosts = 512;
constexpr int MaximumApps = 16384;
using SettingsMap = QMap<QString, QVariant>;

bool integerValue(const QVariant& value, int minimum, int maximum, int& result)
{
    static const QRegularExpression decimal("^-?[0-9]+$");
    if (value.userType() != QMetaType::QString && value.userType() != QMetaType::Int &&
            value.userType() != QMetaType::UInt && value.userType() != QMetaType::LongLong &&
            value.userType() != QMetaType::ULongLong) {
        return false;
    }
    const QString text = value.toString();
    bool valid;
    const qlonglong number = text.toLongLong(&valid);
    if (!valid || !decimal.match(text).hasMatch() || number < minimum || number > maximum) {
        return false;
    }
    result = static_cast<int>(number);
    return true;
}

bool booleanValue(const QVariant& value, bool& result)
{
    if (value.userType() != QMetaType::Bool && value.userType() != QMetaType::QString) {
        return false;
    }
    const QString text = value.toString();
    if (text != "true" && text != "false" && text != "0" && text != "1") {
        return false;
    }
    result = text == "true" || text == "1";
    return true;
}

bool textValue(const QVariant& value, int maximum, QString& result)
{
    if (value.userType() != QMetaType::QString) {
        return false;
    }
    result = value.toString();
    static const QRegularExpression controls("[\\x00-\\x1f\\x7f]");
    return result.size() <= maximum && !controls.match(result).hasMatch();
}

bool pemBytes(const QVariant& value, QByteArray& result)
{
    if (value.userType() != QMetaType::QByteArray && value.userType() != QMetaType::QString) {
        return false;
    }
    result = value.toByteArray();
    if (result.size() > 64 * 1024) {
        return false;
    }
    for (unsigned char character : result) {
        if ((character < 32 && character != '\n' && character != '\r' && character != '\t') || character > 126) {
            return false;
        }
    }
    return true;
}

bool certificateBytes(const QVariant& value, QByteArray& result)
{
    if (!pemBytes(value, result)) {
        return false;
    }
    if (result.isEmpty()) {
        return true;
    }
    const auto certificates = QSslCertificate::fromData(result, QSsl::Pem);
    return certificates.size() == 1 && !certificates.first().isNull() &&
            result.trimmed().startsWith("-----BEGIN CERTIFICATE-----") &&
            result.trimmed().endsWith("-----END CERTIFICATE-----");
}

bool copyPreferences(QSettings& source, SettingsMap& output, int& count)
{
    const QStringList booleans = {
        "unlockbitrate", "autoadjustbitrate", "fullscreen", "vsync", "gameopts", "hostaudio",
        "multicontroller", "hdr", "yuv444", "mdns", "quitAppAfter", "mouseacceleration",
        "abstouchmode", "startwindowed", "framepacing", "connwarnings", "confwarnings",
        "richpresence", "gamepadmouse", "detectnetblocking", "showperfoverlay", "swapmousebuttons",
        "muteonfocusloss", "backgroundgamepad", "reversescroll", "swapfacebuttons", "keepawake"
    };
    for (const QString& key : booleans) {
        if (source.contains(key)) {
            bool value;
            if (!booleanValue(source.value(key), value)) {
                return false;
            }
            output.insert(key, value);
            count++;
        }
    }
    const struct IntegerRule {
        const char* key;
        int minimum;
        int maximum;
    } rules[] = {
        {"width", 1, 16384}, {"height", 1, 16384}, {"fps", 10, 480}, {"bitrate", 500, 500000},
        {"audiocfg", 0, 2}, {"videocfg", 0, 4}, {"videodec", 0, 2}, {"windowmode", 0, 2},
        {"uidisplaymode", 0, 2}, {"capturesyskeys", 0, 2}, {"language", 0, 31},
        {"defaultver", 0, 2}, {"packetsize", 0, 65507}
    };
    for (const auto& rule : rules) {
        const QString key = QString::fromLatin1(rule.key);
        if (source.contains(key)) {
            int value;
            if (!integerValue(source.value(key), rule.minimum, rule.maximum, value) ||
                    (key == "packetsize" && value != 0 && value < 1024)) {
                return false;
            }
            output.insert(key, value);
            count++;
        }
    }
    return true;
}

bool copyHosts(QSettings& source, SettingsMap& output, MoonlightImportResult& result)
{
    int backupCount = 0;
    if (source.contains("hostsbackup/size") &&
            !integerValue(source.value("hostsbackup/size"), 0, MaximumHosts, backupCount)) {
        return false;
    }
    result.usedHostBackup = backupCount > 0;
    const QString array = result.usedHostBackup ? "hostsbackup" : "hosts";
    int hostCount = 0;
    if (source.contains(array + "/size") &&
            !integerValue(source.value(array + "/size"), 0, MaximumHosts, hostCount)) {
        return false;
    }
    QSet<QUuid> hostIds;
    int totalApps = 0;
    for (int hostIndex = 1; hostIndex <= hostCount; hostIndex++) {
        const QString input = array + "/" + QString::number(hostIndex) + "/";
        const QString target = "hosts/" + QString::number(hostIndex) + "/";
        QString hostname;
        QString uuid;
        if (!textValue(source.value(input + "hostname"), 512, hostname) || hostname.isEmpty() ||
                !textValue(source.value(input + "uuid"), 64, uuid) || QUuid(uuid).isNull() || hostIds.contains(QUuid(uuid))) {
            return false;
        }
        hostIds.insert(QUuid(uuid));
        output.insert(target + "hostname", hostname);
        output.insert(target + "uuid", uuid);
        for (const QString& flag : {QString("customname"), QString("nvidiasw")}) {
            bool value = false;
            if (source.contains(input + flag) && !booleanValue(source.value(input + flag), value)) {
                return false;
            }
            output.insert(target + flag, value);
        }
        const QByteArray mac = source.value(input + "mac").toByteArray();
        if (!mac.isEmpty() && mac.size() != 6) {
            return false;
        }
        output.insert(target + "mac", mac);
        for (const QString& prefix : {QString("local"), QString("remote"), QString("manual"), QString("ipv6")}) {
            QString address;
            if (source.contains(input + prefix + "address") &&
                    !textValue(source.value(input + prefix + "address"), 253, address)) {
                return false;
            }
            QHostAddress ip;
            static const QRegularExpression dns("^[A-Za-z0-9_.-]+$");
            if (!address.isEmpty() && !ip.setAddress(address) && !dns.match(address).hasMatch()) {
                return false;
            }
            int port = address.isEmpty() ? 0 : 47989;
            if (source.contains(input + prefix + "port") &&
                    !integerValue(source.value(input + prefix + "port"), address.isEmpty() ? 0 : 1, 65535, port)) {
                return false;
            }
            output.insert(target + prefix + "address", address);
            output.insert(target + prefix + "port", port);
        }
        QByteArray pin;
        if (source.contains(input + "srvcert") && !certificateBytes(source.value(input + "srvcert"), pin)) {
            return false;
        }
        output.insert(target + "srvcert", pin);
        int appCount = 0;
        if (source.contains(input + "apps/size") &&
                !integerValue(source.value(input + "apps/size"), 0, MaximumApps, appCount)) {
            return false;
        }
        totalApps += appCount;
        if (totalApps > MaximumApps) {
            return false;
        }
        output.insert(target + "apps/size", appCount);
        QSet<int> appIds;
        for (int appIndex = 1; appIndex <= appCount; appIndex++) {
            const QString appInput = input + "apps/" + QString::number(appIndex) + "/";
            const QString appTarget = target + "apps/" + QString::number(appIndex) + "/";
            QString name;
            int id;
            if (!textValue(source.value(appInput + "name"), 512, name) || name.isEmpty() ||
                    !integerValue(source.value(appInput + "id"), 0, 2147483647, id) || appIds.contains(id)) {
                return false;
            }
            appIds.insert(id);
            output.insert(appTarget + "name", name);
            output.insert(appTarget + "id", id);
            for (const QString& flag : {QString("hdr"), QString("appcollector"), QString("hidden"), QString("directlaunch")}) {
                bool value = false;
                if (source.contains(appInput + flag) && !booleanValue(source.value(appInput + flag), value)) {
                    return false;
                }
                output.insert(appTarget + flag, value);
            }
        }
    }
    output.insert("hosts/size", hostCount);
    result.hostCount = hostCount;
    return true;
}

bool copyIdentity(QSettings& source, SettingsMap& output)
{
    QByteArray certificate;
    QByteArray key;
    QString uniqueId;
    static const QRegularExpression hexadecimal("^[0-9a-fA-F]{1,16}$");
    if (!certificateBytes(source.value("certificate"), certificate) || certificate.isEmpty() ||
            !pemBytes(source.value("key"), key) || key.isEmpty() ||
            !textValue(source.value("uniqueid"), 16, uniqueId) || !hexadecimal.match(uniqueId).hasMatch()) {
        return false;
    }
    std::unique_ptr<BIO, decltype(&BIO_free)> certBio(BIO_new_mem_buf(certificate.constData(), certificate.size()), BIO_free);
    std::unique_ptr<BIO, decltype(&BIO_free)> keyBio(BIO_new_mem_buf(key.constData(), key.size()), BIO_free);
    if (!certBio || !keyBio) {
        return false;
    }
    std::unique_ptr<X509, decltype(&X509_free)> parsedCert(PEM_read_bio_X509(certBio.get(), nullptr, nullptr, nullptr), X509_free);
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> parsedKey(
        PEM_read_bio_PrivateKey(keyBio.get(), nullptr, [](char*, int, int, void*) { return 0; }, nullptr), EVP_PKEY_free);
    if (!parsedCert || !parsedKey || EVP_PKEY_base_id(parsedKey.get()) != EVP_PKEY_RSA ||
            X509_check_private_key(parsedCert.get(), parsedKey.get()) != 1) {
        return false;
    }
    output.insert("certificate", certificate);
    output.insert("key", key);
    output.insert("uniqueid", uniqueId);
    return true;
}

bool sensitiveKey(const QString& key)
{
    return key == "certificate" || key == "key" || key == "uniqueid" || key.endsWith("/srvcert");
}

QByteArray quotedIni(const QByteArray& bytes)
{
    QByteArray result = "\"";
    for (char character : bytes) {
        switch (character) {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += character; break;
        }
    }
    result += '"';
    return result;
}

bool serialize(const SettingsMap& settings, const QString& directory, QByteArray& contents)
{
    QTemporaryFile metadata(directory + "/.perigee-import-metadata-XXXXXX");
    if (!metadata.open()) {
        return false;
    }
    metadata.close();
    {
        QSettings serialized(metadata.fileName(), QSettings::IniFormat);
        serialized.setFallbacksEnabled(false);
        for (auto entry = settings.cbegin(); entry != settings.cend(); ++entry) {
            if (!sensitiveKey(entry.key())) {
                serialized.setValue(entry.key(), entry.value());
            }
        }
        serialized.sync();
        if (serialized.status() != QSettings::NoError) {
            return false;
        }
    }
    QFile metadataReader(metadata.fileName());
    if (!metadataReader.open(QIODevice::ReadOnly)) {
        return false;
    }
    contents = metadataReader.readAll();
    for (auto entry = settings.cbegin(); entry != settings.cend(); ++entry) {
        if (!sensitiveKey(entry.key())) {
            continue;
        }
        const QString group = entry.key().section('/', 0, 0);
        const bool general = !entry.key().contains('/');
        const QString name = general ? entry.key() : entry.key().mid(group.size() + 1);
        const QByteArray value = entry.key() == "uniqueid" ? entry.value().toString().toLatin1() :
                                                          "@ByteArray(" + entry.value().toByteArray() + ")";
        contents += "\n[" + (general ? QByteArray("General") : group.toUtf8()) + "]\n";
        contents += name.toUtf8().replace('/', '\\') + "=" + quotedIni(value) + "\n";
    }
    return true;
}
}

MoonlightImportResult MoonlightImporter::importSettings(const QString& sourceFile, const QString& destination, bool includePairing)
{
    MoonlightImportResult result;
    QFileInfo sourceInfo(sourceFile);
    if (!sourceInfo.isFile() || sourceInfo.size() > MaximumSourceSize ||
            sourceInfo.canonicalFilePath() == QFileInfo(destination).canonicalFilePath()) {
        result.error = "Source must be an existing settings file, separate from the destination and at most 8 MiB.";
        return result;
    }
    QFile sourceBytes(sourceFile);
    if (!sourceBytes.open(QIODevice::ReadOnly)) {
        result.error = "Unable to read source settings.";
        return result;
    }
    const QByteArray bytes = sourceBytes.read(MaximumSourceSize + 1);
    if (sourceBytes.error() != QFile::NoError || bytes.size() > MaximumSourceSize || bytes.contains('\0') ||
            QString::fromUtf8(bytes).toUtf8() != bytes) {
        result.error = "Source settings must contain valid UTF-8 INI data.";
        return result;
    }
    ProfileLock lock(destination);
    if (!lock.tryLock()) {
        result.error = "Destination profile is in use or its directory cannot be safely locked.";
        return result;
    }
    if (QFileInfo::exists(destination) || QFileInfo(destination).isSymLink()) {
        result.error = "Destination settings already exist; imports never merge or overwrite.";
        return result;
    }
    ProfilePublication snapshot;
    if (!snapshot.prepare(destination, bytes)) {
        result.error = "Filesystem cannot provide anonymous protected staging; no settings were imported.";
        return result;
    }
    QSettings source(snapshot.readPath(), QSettings::IniFormat);
    source.setFallbacksEnabled(false);
    const auto keys = source.allKeys();
    if (source.status() != QSettings::NoError || keys.isEmpty() || keys.size() > 100000) {
        result.error = "Source settings are empty, malformed or exceed supported limits.";
        return result;
    }
    SettingsMap output;
    if (!copyPreferences(source, output, result.preferenceCount) || !copyHosts(source, output, result)) {
        result.error = "Source contains invalid preferences, hosts, application records or server certificates.";
        return result;
    }
    if (includePairing && !copyIdentity(source, output)) {
        result.error = "Pairing import requires a complete readable RSA certificate/key pair and hexadecimal client ID.";
        return result;
    }
    if (result.preferenceCount == 0 && result.hostCount == 0 && !includePairing) {
        result.error = "Source contains no supported preferences or hosts.";
        return result;
    }
    QByteArray contents;
    if (!serialize(output, QFileInfo(destination).absolutePath(), contents)) {
        result.error = "Unable to serialize settings in protected staging storage.";
        return result;
    }
    ProfilePublication publication;
    if (!publication.prepare(destination, contents)) {
        result.error = "Unable to stage imported settings anonymously; no import was committed.";
        return result;
    }
    {
        QSettings verified(publication.readPath(), QSettings::IniFormat);
        verified.setFallbacksEnabled(false);
        if (verified.allKeys() != output.keys() || verified.status() != QSettings::NoError) {
            result.error = "Serialized settings failed round-trip validation; no import was committed.";
            return result;
        }
        for (auto entry = output.cbegin(); entry != output.cend(); ++entry) {
            const bool matches = entry.value().userType() == QMetaType::QByteArray ?
                        verified.value(entry.key()).toByteArray() == entry.value().toByteArray() :
                        verified.value(entry.key()).toString() == entry.value().toString();
            if (!matches) {
                result.error = "Serialized settings changed a value; no import was committed.";
                return result;
            }
        }
    }
    if (!publication.commit()) {
        result.error = "Unable to publish settings atomically without overwriting; no import was committed.";
        return result;
    }
    result.success = true;
    return result;
}

int MoonlightImporter::run(const QStringList& arguments, const QString& destination)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Import selected Moonlight settings into a fresh Perigee profile.\n"
                                     "Optional pairing import shares host-side identity and permissions with Moonlight; unpairing can affect both.");
    parser.addHelpOption();
    parser.addPositionalArgument("import-moonlight", "Import a user-selected Moonlight INI settings file.");
    parser.addOption({"source", "Existing native or portable Moonlight settings file.", "PATH"});
    parser.addOption({"include-pairing", "Explicitly copy the validated client certificate, private key and unique ID."});
    QTextStream standardOutput(stdout);
    QTextStream standardError(stderr);
    if (!parser.parse(arguments)) {
        standardError << "Invalid import arguments. Use 'perigee import-moonlight --help'.\n";
        return 2;
    }
    if (parser.isSet("help")) {
        standardOutput << parser.helpText();
        return 0;
    }
    if (parser.positionalArguments() != QStringList("import-moonlight") || parser.values("source").size() != 1 ||
            parser.value("source").isEmpty() || parser.optionNames().count("include-pairing") > 1) {
        standardError << "Use: perigee import-moonlight --source PATH [--include-pairing]\n";
        return 2;
    }
    const bool includePairing = parser.isSet("include-pairing");
    const auto result = importSettings(parser.value("source"), destination, includePairing);
    if (!result.success) {
        standardError << "Import refused: " << result.error << '\n';
        return 1;
    }
    standardOutput << "Imported " << result.preferenceCount << " preferences and " << result.hostCount << " hosts.\n";
    if (result.usedHostBackup) {
        standardOutput << "Used Moonlight's recoverable host backup without merging host arrays.\n";
    }
    if (includePairing) {
        standardOutput << "Pairing identity copied. Both applications share host-side identity, permissions and possibly virtual displays. "
                          "Unpairing may affect both; concurrent sessions are unverified.\n";
    }
    else {
        standardOutput << "Client credentials were excluded. Pair Perigee as a new client before streaming.\n";
    }
    return 0;
}
