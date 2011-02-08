#include "videoplugin.h"

#include <QtPlugin>

VideoPlugin::VideoPlugin()
{
    setActionList(
                QStringList()
                << "Files"
                << "Addons"
                << "Library");

    m_model = new VideoModel(this);

    setName(tr("Video"));
    setRole("video");
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
