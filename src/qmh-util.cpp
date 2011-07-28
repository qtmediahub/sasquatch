#include <QWidget>
#include "qmh-config.h"

namespace Utils
{
    void optimizeWidgetAttributes(QWidget *widget, bool transparent = false) {
        widget->setAttribute(Qt::WA_OpaquePaintEvent);
        widget->setAutoFillBackground(false);
        if (transparent && Config::isEnabled("shine-through", false))
            widget->setAttribute(Qt::WA_TranslucentBackground);
        else
            widget->setAttribute(Qt::WA_NoSystemBackground);
    }

    QStringList standardResourcePaths(const QString &basePath, const QString &suffix) {
        QStringList paths;
        paths <<  basePath % suffix; // unified repo
        paths << "/usr/share/qtmediahub/" % suffix % "/";
        paths << QDir::homePath() % "/.qtmediahub/" % suffix % "/";
        paths << QDir(Config::value(suffix % "-path", QString(basePath % "/" % suffix))).absolutePath();
        QString envVar("QMH_" % suffix.toUpper() % "PATH");
        if (!qgetenv(envVar.toLatin1().constData()).isEmpty())
            paths << QDir(qgetenv(envVar.toLatin1().constData())).absolutePath();
        return paths;
    }

    QString storageLocation(QDesktopServices::StandardLocation type)
    {
        QString location = QDesktopServices::storageLocation(type);
        return location.isEmpty() ? QString("/tmp") : location;
    }
};
