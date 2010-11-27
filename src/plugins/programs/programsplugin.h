#ifndef PROGRAMSPLUGIN_H
#define PROGRAMSPLUGIN_H

#include <QObject>

#include "qmhplugin.h"

class ProgramsPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)

public:
    ProgramsPlugin() {
        mName = tr("Programs");
        mBrowseable = true;
        mRole = "programs";
    }
    ~ProgramsPlugin() {}
};

#endif // PROGRAMSPLUGIN_H
