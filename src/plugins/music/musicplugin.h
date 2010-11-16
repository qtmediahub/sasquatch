#ifndef MUSICPLUGIN_H
#define MUSICPLUGIN_H

#include <QObject>
#include "qmhplugininterface.h"

class MusicPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
public:
    explicit MusicPlugin(QObject *parent = 0);

signals:

public slots:

};

#endif // MUSICPLUGIN_H
