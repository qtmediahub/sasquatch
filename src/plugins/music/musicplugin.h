#ifndef MUSICPLUGIN_H
#define MUSICPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"

class MusicPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    ~MusicPlugin() {}
    QString name() const { return tr("Music"); }
    bool browseable() const { return true; }
    QString role() const { return "music"; }
};

#endif // MUSICPLUGIN_H
