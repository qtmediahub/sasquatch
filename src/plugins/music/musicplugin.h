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
    Q_PROPERTY(QObject *model READ model NOTIFY modelChanged)

public:
    MusicPlugin();
    ~MusicPlugin() {}

    QObject *pluginProperties() const;

    // accessed from QML
    QObject *model() const { return m_model; }

    void registerPlugin(QDeclarativeContext *context);
signals:
    void modelChanged();

private:
    MusicModel *m_model;
};

#endif // MUSICPLUGIN_H
