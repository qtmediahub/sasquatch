#include "qmlmodulediscovery.h"

#include <QDeclarativeComponent>

QStringList QMLModuleDiscovery::searchPath()
{
    mAvailableItems.clear();
    foreach(const QString dirName, QDir(mPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        QString fileName(mPath + "/" + dirName + "/" + dirName + ".qml");
        QFile prospectiveFile(fileName);
        if(prospectiveFile.exists())
            mAvailableItems << fileName;
    }
}
