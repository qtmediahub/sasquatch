#ifndef VIDEOPLUGIN_H
#define VIDEOPLUGIN_H

#include <QObject>
#include <QDeclarativeContext>
#include "qmhplugininterface.h"
#include "videomodel.h"

class VideoPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)
    Q_PROPERTY(QObject *videoModel READ videoModel NOTIFY videoModelChanged)

public:
    VideoPlugin();
    ~VideoPlugin() {}
    QString name() const { return tr("Video"); }
    bool browseable() const { return true; }
    QString role() const { return "video"; }

    QObject *pluginProperties() const;

    QList<QObject*> childItems() const { return m_childItems; }
    // accessed from QML
    QObject *videoModel() const { return m_model; }

    void registerPlugin(QDeclarativeContext *context);

signals:
    void videoModelChanged();

private:
    QList<QObject *> m_childItems;
    VideoModel *m_model;
};

#endif // VIDEOPLUGIN_H
