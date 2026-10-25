#include "profilestore.h"

#include <QDir>
#include <QFileInfo>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

ProfileLock::ProfileLock(const QString& settingsFile) :
    m_SettingsFile(settingsFile),
    m_Lock(settingsFile + ".profile-lock")
{
    m_Lock.setStaleLockTime(0);
}

bool ProfileLock::tryLock()
{
    QFileInfo directory(QFileInfo(m_SettingsFile).absolutePath());
    const bool existed = directory.exists();
    if (directory.isSymLink() || !QDir().mkpath(directory.absoluteFilePath())) {
        return false;
    }
    if (!existed && !QFile::setPermissions(directory.absoluteFilePath(), QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)) {
        return false;
    }
    struct stat directoryState;
    if (::stat(QFile::encodeName(directory.absoluteFilePath()).constData(), &directoryState) != 0 ||
            directoryState.st_uid != ::geteuid() || (directoryState.st_mode & (S_IWGRP | S_IWOTH))) {
        return false;
    }
    if (QFileInfo(m_SettingsFile + ".profile-lock").isSymLink()) {
        return false;
    }
    return m_Lock.tryLock(0);
}

ProfilePublication::ProfilePublication() :
    m_DirectoryFd(-1),
    m_Prepared(false)
{
}

ProfilePublication::~ProfilePublication()
{
    if (m_DirectoryFd >= 0) {
        ::close(m_DirectoryFd);
    }
}

bool ProfilePublication::prepare(const QString& destination, const QByteArray& contents)
{
    if (m_DirectoryFd >= 0) {
        return false;
    }
    QFileInfo target(destination);
    m_FileName = QFile::encodeName(target.fileName());
    m_DirectoryFd = ::open(QFile::encodeName(target.absolutePath()).constData(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (m_DirectoryFd < 0) {
        return false;
    }
    const int descriptor = ::openat(m_DirectoryFd, ".", O_TMPFILE | O_RDWR | O_CLOEXEC, 0600);
    if (descriptor < 0) {
        return false;
    }
    if (::fchmod(descriptor, 0600) != 0) {
        ::close(descriptor);
        return false;
    }
    if (!m_File.open(descriptor, QIODevice::WriteOnly, QFileDevice::AutoCloseHandle)) {
        ::close(descriptor);
        return false;
    }
    m_Prepared = m_File.write(contents) == contents.size() && m_File.flush() && ::fsync(descriptor) == 0;
    return m_Prepared;
}

bool ProfilePublication::commit()
{
    if (!m_Prepared || !m_File.isOpen()) {
        return false;
    }
    const QByteArray descriptorPath = QByteArray("/proc/self/fd/") + QByteArray::number(m_File.handle());
    if (::linkat(AT_FDCWD, descriptorPath.constData(), m_DirectoryFd, m_FileName.constData(), AT_SYMLINK_FOLLOW) != 0) {
        return false;
    }
    if (::fsync(m_DirectoryFd) != 0) {
        qWarning("Imported profile was published, but directory durability synchronization failed");
    }
    m_File.close();
    return true;
}

QString ProfilePublication::readPath() const
{
    return m_Prepared ? QString("/proc/self/fd/%1").arg(m_File.handle()) : QString();
}
