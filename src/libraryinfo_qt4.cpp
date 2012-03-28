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
#if defined(Q_WS_QWS)
    return QString("/tmp");
#else
    return storageLocation(QDesktopServices::TempLocation);
#endif	//#if defined(Q_WS_QWS)
}

QString LibraryInfo::logPath()
{
#if defined(Q_WS_QWS)
    return QString("/tmp");
#else
    return storageLocation(QDesktopServices::TempLocation);
#endif	//#if defined(Q_WS_QWS)
}


