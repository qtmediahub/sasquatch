#ifndef MEDIAPLUGIN_H
#define MEDIAPLUGIN_H

#include <QObject>
#include <QStringList>
#include "global.h"

class MediaParser;

class QMH_EXPORT MediaPlugin : public QObject
{
    Q_OBJECT

public:
    MediaPlugin() { }
    virtual ~MediaPlugin() { }

    virtual QStringList parserKeys() const = 0;
    virtual MediaParser *createParser(const QString &key) = 0;
};

Q_DECLARE_INTERFACE(MediaPlugin, "com.qtmediahub.MediaPlugin/1.0")

#endif // MEDIAPLUGIN_H
