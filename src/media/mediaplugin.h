#ifndef MEDIAPLUGIN_H
#define MEDIAPLUGIN_H

#include <QObject>
#include <QStringList>
#include <QMetaEnum>
#include "global.h"

class QDeclarativeContext;

class QMH_EXPORT MediaPlugin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    MediaPlugin(const QString &name, QObject *parent = 0)
        : QObject(parent), m_name(name)
    {}

    virtual ~MediaPlugin() { /*no impl*/ }

    QString name() const { return m_name; }

private:
    QString m_name;
};

Q_DECLARE_INTERFACE(MediaPlugin, "com.qtmediahub.MediaPlugin/1.0")

#endif // MEDIAPLUGIN_H
