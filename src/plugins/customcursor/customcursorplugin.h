#ifndef CUSTOMCURSORPLUGIN_H
#define CUSTOMCURSORPLUGIN_H

#include <QObject>
#include <QDeclarativeItem>

#include "qmhplugininterface.h"
#include "customcursor.h"

class CustomCursorPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    ~CustomCursorPlugin() {}
    QString name() const { return tr("CustomCursor"); }
    bool browseable() const { return true; }
    QString role() const { return ""; }

    void registerPlugin(QDeclarativeContext *context) { Q_UNUSED(context); qmlRegisterType<CustomCursor>("CustomCursor", 1, 0, "CustomCursor"); }
};

#endif // CUSTOMCURSORPLUGIN_H
