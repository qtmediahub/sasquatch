#include <QtPlugin>
\
#include "animatedtilesplugin.h"
#include "animatedtilesitem.h"


AnimatedTilesPlugin::AnimatedTilesPlugin()
{
    setName("AnimatedTiles");
    //This should not be an advertized plugin until it is implemented
    //setRole("animatedTiles");
}


Q_EXPORT_PLUGIN2(animatedTiles, AnimatedTilesPlugin)

void AnimatedTilesPlugin::registerPlugin(QDeclarativeContext *context)
{
    qmlRegisterType<AnimatedTilesItem>("AnimatedTiles", 1, 0, "AnimatedTiles");
}
