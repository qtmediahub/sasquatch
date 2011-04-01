#include "videoplugin.h"

#include <QtPlugin>

VideoPlugin::VideoPlugin()
{
    mActionList << "Files"
                << "Addons"
                << "Library";

    m_model = new VideoModel(this);

    mName = tr("Video");
    mRole = "video";
}

QObject *VideoPlugin::pluginProperties() const
{
    return const_cast<VideoPlugin *>(this);
}

void VideoPlugin::registerPlugin(QDeclarativeContext *context)
{
    //FIXME: these structures should be suitably disconnected from QML to be usable from html
    //Post CES adjustment :)
    if (context)
        m_model->registerImageProvider(context);
}

Q_EXPORT_PLUGIN2(video, VideoPlugin)
