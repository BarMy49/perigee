#pragma once

#include <QFile>
#include <QLockFile>

class ProfileLock
{
public:
    explicit ProfileLock(const QString& settingsFile);
    bool tryLock();

private:
    QString m_SettingsFile;
    QLockFile m_Lock;
};

class ProfilePublication
{
public:
    ProfilePublication();
    ~ProfilePublication();
    ProfilePublication(const ProfilePublication&) = delete;
    ProfilePublication& operator=(const ProfilePublication&) = delete;

    bool prepare(const QString& destination, const QByteArray& contents);
    QString readPath() const;
    bool commit();

private:
    QFile m_File;
    int m_DirectoryFd;
    QByteArray m_FileName;
    bool m_Prepared;
};
