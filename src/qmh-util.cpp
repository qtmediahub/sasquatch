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

        // TODO check below two paths if still valid maybe basePath is wrong as the default value
        paths <<  basePath % QString::fromLatin1("/") % suffix; // unified repo
        paths <<  basePath % QString::fromLatin1("/../../") % suffix; // submodule repo

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

    QString storageLocation(QDesktopServices::StandardLocation type)
    {
        QString location = QDesktopServices::storageLocation(type);
        return location.isEmpty() ? QString("/tmp") : location;
    }
};
