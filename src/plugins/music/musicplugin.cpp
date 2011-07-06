#include "musicplugin.h"
#include "musicparser.h"

#include <QtGui>
#include <QtDeclarative>

MusicPlugin::MusicPlugin()
    : QMHPlugin(tr("Music"), Music)
{
    MusicParser *parser = new MusicParser;
    QMetaObject::invokeMethod(MediaScanner::instance(), "addParser", Qt::QueuedConnection, 
                              Q_ARG(MediaParser *, parser));
}

Q_EXPORT_PLUGIN2(music, MusicPlugin)
