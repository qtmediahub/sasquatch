#ifndef SCRIPTSPLUGIN_H
#define SCRIPTSPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"

class ScriptsPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    ~ScriptsPlugin() {}
    QString name() const { return tr("Scripts"); }
    bool browseable() const { return true; }
    QString role() const { return "scripts"; }
};

#endif // SCRIPTSPLUGIN_H
