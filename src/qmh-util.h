#ifndef QMHUTIL_H
#define QMHUTIL_H

namespace Utils
{
    static void optimizeWidgetAttributes(QWidget *widget, bool transparent = false);
    static const QStringList standardResourcePaths(const QString &basePath, const QString &suffix);
    static QString storageLocation(QDesktopServices::StandardLocation type);
};

#endif // QMHUTIL_H
