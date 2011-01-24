#include <QtPlugin>
\
#include "animatedtilesplugin.h"
#include "animatedtilesitem.h"


AnimatedTilesPlugin::AnimatedTilesPlugin()
{
    this->mName = "AnimatedTiles";
    //This should not be an advertized plugin until it is implemented
    //this->mRole = "animatedTiles";
}


Q_EXPORT_PLUGIN2(animatedTiles, AnimatedTilesPlugin)

void AnimatedTilesPlugin::registerPlugin(QDeclarativeContext *context)
{
    qmlRegisterType<AnimatedTilesItem>("AnimatedTiles", 1, 0, "AnimatedTiles");
}
