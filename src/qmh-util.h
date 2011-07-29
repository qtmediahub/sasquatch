#ifndef QMHUTIL_H
#define QMHUTIL_H

namespace Utils
{
    void optimizeWidgetAttributes(QWidget *widget, bool transparent = false);
    const QStringList standardResourcePaths(const QString &basePath, const QString &suffix);
    QString storageLocation(QDesktopServices::StandardLocation type);
};

#endif // QMHUTIL_H
