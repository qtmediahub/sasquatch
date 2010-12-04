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
}

QObject *VideoPlugin::pluginProperties() const
{
    return const_cast<VideoPlugin *>(this);
}


Q_EXPORT_PLUGIN2(video, VideoPlugin)

#include "videoplugin.moc"
