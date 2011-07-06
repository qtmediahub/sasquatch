#include "videoplugin.h"
#include "videoparser.h"

#include <QtPlugin>

VideoPlugin::VideoPlugin()
    : QMHPlugin(tr("Video"), Video)
{
    VideoParser *parser = new VideoParser;
    QMetaObject::invokeMethod(MediaScanner::instance(), "addParser", Qt::QueuedConnection, 
                              Q_ARG(MediaParser *, parser));
}

Q_EXPORT_PLUGIN2(video, VideoPlugin)
