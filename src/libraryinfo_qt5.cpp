#include "libraryinfo.h"

#include <QStandardPaths>

static QString storageLocation(QStandardPaths::StandardLocation type)
{
    QString location = QStandardPaths::writableLocation(type);
    return location.isEmpty() ? QString("/tmp") : location;
}

QString LibraryInfo::dataPath()
{
    return storageLocation(QStandardPaths::DataLocation);
}

QString LibraryInfo::tempPath()
{
    return storageLocation(QStandardPaths::TempLocation);
}

QString LibraryInfo::logPath()
{
    return storageLocation(QStandardPaths::TempLocation);
}


