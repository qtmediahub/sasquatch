#ifndef VIDEOPLUGIN_H
#define VIDEOPLUGIN_H

#include <QObject>
#include <QDeclarativeContext>
#include "qmhplugin.h"
#include "videomodel.h"

class VideoPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)
    Q_PROPERTY(QObject *model READ model NOTIFY modelChanged)

public:
    VideoPlugin();
    ~VideoPlugin() {}

    QObject *pluginProperties() const;

    // accessed from QML
    QObject *model() const { return m_model; }

    void registerPlugin(QDeclarativeContext *context);

signals:
    void modelChanged();

private:
    VideoModel *m_model;
};

#endif // VIDEOPLUGIN_H
