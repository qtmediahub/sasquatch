#include "qmlmodulediscovery.h"

#include <QDeclarativeComponent>

void QMLModuleDiscovery::searchPath()
{
    mAvailableItems.clear();
    foreach(const QString dirName, QDir(mPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        QString fileName(mPath + "/" + dirName + "/" + dirName + ".qml");
        QFile prospectiveFile(fileName);
        if(prospectiveFile.exists())
            mAvailableItems << fileName;
    }
}
