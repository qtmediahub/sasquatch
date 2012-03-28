#include "contextcontentrpc.h"

#include <QDebug>

ContextContentRpc::ContextContentRpc(QObject *parent) :
    QObject(parent)
{
    setObjectName("contextContent");
}

void ContextContentRpc::newContextContent(const QString &skinName, const QString &contentName, QList<int> idList)
{
    QString sn = skinName;
    QString cN = contentName;
    emit sendNewContextContent(sn, cN, idList);
}
