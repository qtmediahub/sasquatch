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

MusicPlugin::MusicPlugin()
{
    m_childItems << new MusicPluginItem(tr("Files"), this)
                 << new MusicPluginItem(tr("Add-ons"), this)
                 << new MusicPluginItem(tr("Library"), this);

    m_model = new MediaModel(MediaModel::Music, this);
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
    m_model->registerImageProvider(context);
}

Q_EXPORT_PLUGIN2(music, MusicPlugin)

#include "musicplugin.moc"

