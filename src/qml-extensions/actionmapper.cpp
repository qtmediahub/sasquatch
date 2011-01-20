#include "actionmapper.h"

#include "backend.h"

ActionMapper::ActionMapper(QObject *p)
    : QObject(p),
      mapName("stdkeyboard"),
      mapPath(Backend::instance()->resourcePath() + "/devices/keymaps/")
{
    maps = QDir(mapPath).entryList(QDir::Files);
    qWarning() << "Available keyboard maps" << maps;
}

void ActionMapper::populateMap()
{
    const QMetaObject &ActionMO = ActionMapper::staticMetaObject;
    int enumIndex = ActionMO.indexOfEnumerator("Action");
    QMetaEnum actionEnum = ActionMO.enumerator(enumIndex);

    enumIndex = staticQtMetaObject.indexOfEnumerator("Key");
    QMetaEnum keyEnum = staticQtMetaObject.enumerator(enumIndex);

    QFile mapFile(mapPath + mapName);
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
        mapName = "stdkeyboard";
        populateMap();
    }
}
