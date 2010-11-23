#include "dashboard.h"

#include <QDeclarativeComponent>

Dashboard::Dashboard(QDeclarativeItem *parent) :
    QDeclarativeItem(parent)
{
}

QStringList Dashboard::discoverWidgets()
{
    QStringList availableWidgets;
    foreach(const QString dirName, QDir(mWidgetPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        QString fileName(mWidgetPath + "/" + dirName + "/" + dirName + ".qml");
        QFile prospectiveFile(fileName);
        if(prospectiveFile.exists())
            availableWidgets << fileName;
    }
    return availableWidgets;
}
