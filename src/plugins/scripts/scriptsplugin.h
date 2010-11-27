#ifndef SCRIPTSPLUGIN_H
#define SCRIPTSPLUGIN_H

#include <QObject>

#include "qmhplugin.h"

class ScriptsPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)

public:
    ScriptsPlugin() {
        mName = tr("Scripts");
        mBrowseable = true;
        mRole = "scripts";
    }

    ~ScriptsPlugin() {}
};

#endif // SCRIPTSPLUGIN_H
