#ifndef PROGRAMSPLUGIN_H
#define PROGRAMSPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"

class ProgramsPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    ~ProgramsPlugin() {}
    QString name() const { return tr("Programs"); }
    bool browseable() const { return true; }
    QString role() const { return "programs"; }
};

#endif // PROGRAMSPLUGIN_H
