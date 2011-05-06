#include "videoplugin.h"

#include <QtPlugin>

VideoPlugin::VideoPlugin()
    : QMHPlugin(tr("Video"), Video)
{
    m_model = new VideoModel(this);
}

void VideoPlugin::registerPlugin(QDeclarativeContext *context)
{
    if (context) {
        context->engine()->addImageProvider("videomodel", new VideoModelImageProvider);
    }
}

Q_EXPORT_PLUGIN2(video, VideoPlugin)
