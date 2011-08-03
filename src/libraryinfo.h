#ifndef LIBRARYINFO_H
#define LIBRARYINFO_H

#include <QString>
#include <QDesktopServices>
#include "global.h"

class QMH_EXPORT LibraryInfo
{
public:
    static QString basePath();
    static QString translationPath();
    static QString pluginPath();
    static QString resourcePath();
    static QString thumbnailPath();
    static QStringList skinPaths();
    static QStringList applicationPaths();
    static QString dataPath();
    static QString tempPath();
    static QString logPath();

private:
    LibraryInfo();
};

#endif // LIBRARYINFO_H

