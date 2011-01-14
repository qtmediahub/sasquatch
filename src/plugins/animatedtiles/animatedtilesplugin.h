#ifndef ANIMATEDTILESPLUGIN_H
#define ANIMATEDTILESPLUGIN_H

#include "qmhplugininterface.h"

class AnimatedTilesPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    AnimatedTilesPlugin();

    void registerPlugin(QDeclarativeContext *context);

};

#endif // ANIMATEDTILESPLUGIN_H
