#ifndef MUSICPLUGIN_H
#define MUSICPLUGIN_H

#include <QObject>
#include "qmhplugininterface.h"

class MusicPlugin : public QMHPluginInterface
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(bool browseable READ browseable)
    Q_INTERFACES(QMHPluginInterface)
public:
    ~MusicPlugin() {}
    QString name() const { return tr("Music"); }
    bool browseable() const { return true; }
};

#endif // MUSICPLUGIN_H
