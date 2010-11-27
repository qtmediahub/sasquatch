#ifndef MUSICPLUGIN_H
#define MUSICPLUGIN_H

#include <QObject>

#include "qmhplugin.h"

class MusicPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)

public:
    MusicPlugin();
    ~MusicPlugin() {}
};

#endif // MUSICPLUGIN_H
