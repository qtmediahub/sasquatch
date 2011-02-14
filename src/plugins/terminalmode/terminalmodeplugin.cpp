#include <QtPlugin>

#include "terminalmodeplugin.h"
#include "qtmdeclarativeitem.h"


TerminalModePlugin::TerminalModePlugin()
{
    setName("TerminalMode");
    //This should not be an advertized plugin until it is implemented
//    setRole("terminalMode");

//    qmlRegisterType<AnimatedTilesItem>("AnimatedTiles", 1, 0, "AnimatedTiles");
    QTmDeclarativeItem::initModule();
}


Q_EXPORT_PLUGIN2(terminalMode, TerminalModePlugin)
