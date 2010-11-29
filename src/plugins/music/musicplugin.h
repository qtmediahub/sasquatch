#ifndef MUSICPLUGIN_H
#define MUSICPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"

class MusicPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    MusicPlugin();
    ~MusicPlugin() {}
    QString name() const { return tr("Music"); }
    bool browseable() const { return true; }
    QString role() const { return "music"; }

    QList<QObject*> childItems() const;

private:
    QList<QObject *> m_childItems;
};

#endif // MUSICPLUGIN_H
