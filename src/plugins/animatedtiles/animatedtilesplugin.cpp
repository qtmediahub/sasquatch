#include <QtPlugin>

#include "animatedtilesplugin.h"
#include "animatedtilesitem.h"


AnimatedTilesPlugin::AnimatedTilesPlugin()
    : QMHPlugin(tr("AnimatedTiles"), Application)
{
    qmlRegisterType<AnimatedTilesItem>("AnimatedTiles", 1, 0, "AnimatedTiles");
}

Q_EXPORT_PLUGIN2(animatedTiles, AnimatedTilesPlugin)
