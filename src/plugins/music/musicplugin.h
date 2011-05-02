#ifndef MUSICPLUGIN_H
#define MUSICPLUGIN_H

#include <QObject>

#include "qmhplugin.h"
#include "musicmodel.h"

class QDeclarativeContext;

class MusicPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)
    Q_PROPERTY(QObject *model READ model NOTIFY modelChanged)

public:
    MusicPlugin();
    ~MusicPlugin() {}

    // accessed from QML
    QObject *model() const { return m_model; }

    void registerPlugin(QDeclarativeContext *context);
signals:
    void modelChanged();

private:
    MusicModel *m_model;
};

#endif // MUSICPLUGIN_H
