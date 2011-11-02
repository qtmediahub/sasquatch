#include "libraryinfo.h"

#include <QDesktopServices>

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


