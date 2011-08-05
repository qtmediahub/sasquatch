#include "libraryinfo.h"
#include "qmh-config.h"

#include <QDesktopServices>

static QStringList standardResourcePaths(const QString &suffix)
{
    QString basePath = LibraryInfo::basePath();
    QStringList paths;

    // TODO check below two paths if still valid maybe basePath is wrong as the default value
    paths <<  basePath % QString::fromLatin1("/") % suffix; // unified repo
#ifdef Q_OS_MAC
    paths <<  basePath % QString::fromLatin1("/../../../") % suffix; // submodule repo
#else
    paths <<  basePath % QString::fromLatin1("/../../") % suffix; // submodule repo
#endif

    paths << "" QMH_INSTALL_PREFIX % QString::fromLatin1("/") % suffix % QString::fromLatin1("/");
    paths << QDir::homePath() % "/.qtmediahub/" % suffix % QString::fromLatin1("/");

    QString configPath = Config::value(suffix % "-path", QString());
    if (!configPath.isEmpty())
        paths << QDir(configPath).absolutePath();

    QString envVar("QMH_" % suffix.toUpper() % "PATH");
    if (!qgetenv(envVar.toLatin1().constData()).isEmpty())
        paths << QDir(qgetenv(envVar.toLatin1().constData())).absolutePath();

    return paths;
}

static QString storageLocation(QDesktopServices::StandardLocation type)
{
    QString location = QDesktopServices::storageLocation(type);
    return location.isEmpty() ? QString("/tmp") : location;
}

QString LibraryInfo::dataPath()
{
    return storageLocation(QDesktopServices::DataLocation);
}

QString LibraryInfo::tempPath()
{
    return storageLocation(QDesktopServices::TempLocation);
}

QString LibraryInfo::logPath()
{
    return storageLocation(QDesktopServices::TempLocation);
}

QString LibraryInfo::basePath()
{
    static QString cachedBasePath;
    if (!cachedBasePath.isEmpty())
        return cachedBasePath;

#ifdef Q_OS_MAC
    QString platformOffset("/../../../");
#else
    QString platformOffset;
#endif

    QString defaultBasePath(QMH_INSTALL_PREFIX);
    if (Config::value("testing", false) || !QDir(defaultBasePath).exists()) {
        qDebug() << "Either testing or uninstalled: running in build dir";
        defaultBasePath = QCoreApplication::applicationDirPath() + platformOffset;
    }
    cachedBasePath = Config::value("base-path",  defaultBasePath);
    return cachedBasePath;
}

QString LibraryInfo::translationPath()
{
    return basePath() % "/translations/";
}

QString LibraryInfo::pluginPath()
{
    return QDir(Config::value("plugins-path", QString(basePath() % "/plugins"))).absolutePath();
}

QString LibraryInfo::resourcePath()
{
    if (!qgetenv("QMH_RESOURCEPATH").isEmpty())
        return QDir(qgetenv("QMH_RESOURCEPATH")).absolutePath();
    else
        return QDir(Config::value("resources-path", QString(basePath() % "/resources"))).absolutePath();
}

QString LibraryInfo::thumbnailPath()
{
    return Config::value("thumbnail-path", QDir::homePath() + "/.thumbnails/" + QApplication::applicationName() + "/");
}

QStringList LibraryInfo::skinPaths()
{
    return standardResourcePaths("skins");
}

QStringList LibraryInfo::applicationPaths()
{
    return standardResourcePaths("apps");
}

QString LibraryInfo::databaseFilePath()
{
    return LibraryInfo::dataPath() + "/media.db";
}

