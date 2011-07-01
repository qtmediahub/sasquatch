#include "videoplugin.h"
#include "videoparser.h"

#include <QtPlugin>

VideoPlugin::VideoPlugin()
    : QMHPlugin(tr("Video"), Video)
{
    VideoParser *parser = new VideoParser;
    MediaScanner::instance()->addParser(parser);
}

Q_EXPORT_PLUGIN2(video, VideoPlugin)
