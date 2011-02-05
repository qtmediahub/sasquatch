#include "actionmapper.h"

#include "backend.h"
#include "frontend.h"

class Frontend;

ActionMapper::ActionMapper(Frontend *p)
    : QObject(p),
      pFrontend(p),
      mapName(""),
      mapPath(Backend::instance()->resourcePath() + "/devices/keymaps/")
{
    setObjectName("qmhrpc");

    maps = QDir(mapPath).entryList(QDir::Files);
    qWarning() << "Available keyboard maps" << maps;
    populateMap();
}

void ActionMapper::takeAction(Action action)
{
    if (!pFrontend || !keyHash.contains(action))
        return;
    QKeyEvent keyPress(QEvent::KeyPress, keyHash[action].at(0), Qt::NoModifier);
    qApp->sendEvent(pFrontend->centralWidget(), &keyPress);
    QKeyEvent keyRelease(QEvent::KeyRelease, keyHash[action].at(0), Qt::NoModifier);
    qApp->sendEvent(pFrontend->centralWidget(), &keyRelease);
}

void ActionMapper::populateMap()
{
    keyHash.clear();
    loadMapFromDisk(mapPath + "stdkeyboard");
    if (!mapName.isEmpty())
        loadMapFromDisk(mapPath + mapName);
}

void ActionMapper::loadMapFromDisk(const QString &mapFilePath)
{
    const QMetaObject &ActionMO = ActionMapper::staticMetaObject;
    int enumIndex = ActionMO.indexOfEnumerator("Action");
    QMetaEnum actionEnum = ActionMO.enumerator(enumIndex);

    enumIndex = staticQtMetaObject.indexOfEnumerator("Key");
    QMetaEnum keyEnum = staticQtMetaObject.enumerator(enumIndex);

    QFile mapFile(mapFilePath);
    if (mapFile.exists()
        && mapFile.open(QIODevice::ReadOnly))
    {
        QTextStream mapStream(&mapFile);
        while(!mapStream.atEnd())
        {
            QStringList mapping = mapStream.readLine().split("=");
            QStringList keyStrings = mapping.at(1).split(",");
            QList<int> keys;
            foreach(const QString &key, keyStrings)
            {
                int keyIndex = keyEnum.keyToValue(key.toAscii().constData());
                if(keyIndex == -1)
                    qWarning() << "Key does not exist" << key;
                else
                    keys << keyIndex;
            }
            int index =
                actionEnum.keyToValue(mapping[0].toAscii().constData());
            if (index == -1)
                qWarning() << "Mapped action is not defined in ActionMapper, skipping: " << mapping[0];
            else
                keyHash[static_cast<Action>(index)] = keys;
        }
        if(actionEnum.keyCount() != keyHash.size())
            qWarning("Certain actions have not been mapped");
        else
            qWarning("All keys mapped correctly");
    } else {
        qWarning() << "Could not load extended map: " << mapFilePath;
    }
}
