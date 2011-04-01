#ifndef CUSTOMCURSORPLUGIN_H
#define CUSTOMCURSORPLUGIN_H

#include <QObject>
#include <QDeclarativeItem>
#include <QDeclarativeContext>

#include "qmhplugin.h"
#include "customcursor.h"
#include "qmh-config.h"

class CustomCursorPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)

public:
    CustomCursorPlugin() {
        mName = tr("CustomCursor");
        mAdvertized = false;
    }
    ~CustomCursorPlugin() {}

    void registerPlugin(QDeclarativeContext *context) { context->setContextProperty("cursor", new CustomCursor(this)); }
};

#endif // CUSTOMCURSORPLUGIN_H
