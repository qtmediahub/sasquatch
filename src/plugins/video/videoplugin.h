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
    Q_PROPERTY(QObject *model READ model NOTIFY modelChanged)

public:
    VideoPlugin();
    ~VideoPlugin() {}

    QObject *pluginProperties() const;

    QList<QObject*> childItems() const { return m_childItems; }
    // accessed from QML
    QObject *model() const { return m_model; }

    void registerPlugin(QDeclarativeContext *context);

signals:
    void modelChanged();

private:
    QList<QObject *> m_childItems;
    VideoModel *m_model;
};

#endif // VIDEOPLUGIN_H
