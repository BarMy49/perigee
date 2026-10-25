#pragma once

#include <QStringList>

struct MoonlightImportResult
{
    bool success = false;
    int preferenceCount = 0;
    int hostCount = 0;
    bool usedHostBackup = false;
    QString error;
};

class MoonlightImporter
{
public:
    static MoonlightImportResult importSettings(const QString& source, const QString& destination, bool includePairing);
    static int run(const QStringList& arguments, const QString& destination);
};
