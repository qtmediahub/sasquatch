#ifndef ANIMATEDTILESPLUGIN_H
#define ANIMATEDTILESPLUGIN_H

#include "qmhplugininterface.h"

class AnimatedTilesPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    AnimatedTilesPlugin();
};

#endif // ANIMATEDTILESPLUGIN_H
