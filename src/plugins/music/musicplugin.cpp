#include "musicplugin.h"

#include <QtGui>
#include <QtDeclarative>

class MusicPluginItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    MusicPluginItem(const QString &name, QObject *parent = 0) : QObject(parent), m_name(name) { }

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

class Files : public QObject
{
    Q_OBJECT
public:
    Files(QObject *parent = 0) : QObject(parent) { }
};

class ImageProvider : public QDeclarativeImageProvider
{
public:
    ImageProvider(MusicPlugin *plugin) 
        : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap),
          m_plugin(plugin)
    {
    }

    QImage  requestImage(const QString &id, QSize *size, const QSize &requestedSize)
    {
        if (id.startsWith("musicmodel")) {
            return m_plugin->m_model->decorationImage(id.mid(10), size, requestedSize);
        }
        return QImage();
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
    {
        if (id.startsWith("musicmodel")) {
            return m_plugin->m_model->decorationPixmap(id.mid(10), size, requestedSize);
        }
        return QPixmap();
    }

private:
    MusicPlugin *m_plugin;
};

MusicPlugin::MusicPlugin()
{
    m_childItems << new MusicPluginItem(tr("Files"), this)
                 << new MusicPluginItem(tr("Add-ons"), this)
                 << new MusicPluginItem(tr("Library"), this);

    m_model = new MediaModel(this);
}

QList<QObject*> MusicPlugin::childItems() const
{
    return m_childItems;
}

 
QObject *MusicPlugin::pluginProperties() const
{
    return const_cast<MusicPlugin *>(this);
}

void MusicPlugin::registerPlugin(QDeclarativeContext *context)
{
    context->engine()->addImageProvider("qtmediahub", new ImageProvider(this));
}

Q_EXPORT_PLUGIN2(music, MusicPlugin)

#include "musicplugin.moc"

