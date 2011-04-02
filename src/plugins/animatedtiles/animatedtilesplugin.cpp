#include <QtPlugin>

#include "animatedtilesplugin.h"
#include "animatedtilesitem.h"


AnimatedTilesPlugin::AnimatedTilesPlugin()
{
    mName = tr("AnimatedTiles");
    mRole = Application;

    qmlRegisterType<AnimatedTilesItem>("AnimatedTiles", 1, 0, "AnimatedTiles");
}

Q_EXPORT_PLUGIN2(animatedTiles, AnimatedTilesPlugin)
