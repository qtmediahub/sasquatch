#include "contextcontentrpc.h"

#include <QDebug>

ContextContentRpc::ContextContentRpc(QObject *parent) :
    QObject(parent)
{
}

void ContextContentRpc::newContextContent(const QString &skinName, const QString &contentName, QList<int> idList)
{
    qDebug() << "void ContextContentRpc::newContextContent(const QString &skinName, const QString &contentName)";
    QString sn = skinName;
    QString cN = contentName;
    emit sendNewContextContent(sn, cN, idList);
}
