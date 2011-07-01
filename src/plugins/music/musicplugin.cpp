#include "musicplugin.h"
#include "musicparser.h"

#include <QtGui>
#include <QtDeclarative>

MusicPlugin::MusicPlugin()
    : QMHPlugin(tr("Music"), Music)
{
    MusicParser *parser = new MusicParser;
    MediaScanner::instance()->addParser(parser);
}

Q_EXPORT_PLUGIN2(music, MusicPlugin)
