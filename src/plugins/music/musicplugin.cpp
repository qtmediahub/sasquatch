#include "musicplugin.h"

#include <QtPlugin>

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

}

QList<QObject*> MusicPlugin::childItems() const
{
    return m_childItems;
}

Q_EXPORT_PLUGIN2(music, MusicPlugin)

#include "musicplugin.moc"

