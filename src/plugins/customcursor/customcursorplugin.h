#ifndef CUSTOMCURSORPLUGIN_H
#define CUSTOMCURSORPLUGIN_H

#include <QObject>
#include <QDeclarativeItem>

#include "qmhplugininterface.h"
#include "customcursor.h"
#include "qmh-config.h"

class CustomCursorPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    CustomCursorPlugin() {
        setName(tr("CustomCursor"));
        setRole("");
    }
    ~CustomCursorPlugin() {}

    void registerPlugin(QDeclarativeContext *context) { Q_UNUSED(context); qmlRegisterType<CustomCursor>("CustomCursor", 1, 0, "CustomCursor"); }
};

#endif // CUSTOMCURSORPLUGIN_H
