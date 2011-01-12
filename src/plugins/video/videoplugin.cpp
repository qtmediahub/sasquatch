#include "videoplugin.h"

#include <QtPlugin>

#include "submenuentry.h"

class VideoPluginItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    VideoPluginItem(const QString &name, QObject *parent = 0) : QObject(parent), m_name(name) { }

    QString name() const {
        return m_name;
    }

    void setName(const QString &name) {
        m_name = name;
        emit nameChanged();
    }
signals:
    void nameChanged();

private:
    QString m_name;
};

VideoPlugin::VideoPlugin()
{
    m_childItems << new VideoPluginItem(tr("Files"), this)
                 << new VideoPluginItem(tr("Add-ons"), this)
                 << new VideoPluginItem(tr("Library"), this);

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

#include "videoplugin.moc"
