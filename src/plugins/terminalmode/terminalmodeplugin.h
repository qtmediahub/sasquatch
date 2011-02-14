#ifndef TerminalModePluginPLUGIN_H
#define TerminalModePluginPLUGIN_H

#include "qmhplugininterface.h"

class TerminalModePlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    TerminalModePlugin();
};

#endif // ANIMATEDTILESPLUGIN_H
