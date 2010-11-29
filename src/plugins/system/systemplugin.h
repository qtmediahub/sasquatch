#ifndef SYSTEMPLUGIN_H
#define SYSTEMPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"

class SystemPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    ~SystemPlugin() {}
    QString name() const { return tr("System"); }
    bool browseable() const { return false; }
    QString role() const { return "system"; }
};

#endif // SYSTEMPLUGIN_H
