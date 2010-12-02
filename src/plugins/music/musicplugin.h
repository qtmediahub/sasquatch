#ifndef MUSICPLUGIN_H
#define MUSICPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"
#include "musicmodel.h"

class QDeclarativeContext;

class MusicPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)
    Q_PROPERTY(QObject *musicModel READ musicModel NOTIFY musicModelChanged)

public:
    MusicPlugin();
    ~MusicPlugin() {}
    QString name() const { return tr("Music"); }
    bool browseable() const { return true; }
    QString role() const { return "music"; }

    QList<QObject*> childItems() const;

    QObject *pluginProperties() const;

    // accessed from QML
    QObject *musicModel() const { return m_model; }

    void registerPlugin(QDeclarativeContext *context);
signals:
    void musicModelChanged();

private:
    QList<QObject *> m_childItems;
    MusicModel *m_model;

    friend class ImageProvider;
};

#endif // MUSICPLUGIN_H
