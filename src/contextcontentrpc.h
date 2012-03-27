#ifndef CONTEXTCONTENTRPC_H
#define CONTEXTCONTENTRPC_H

#include <QObject>

class ContextContentRpc : public QObject
{
    Q_OBJECT
public:
    explicit ContextContentRpc(QObject *parent = 0);

    Q_SCRIPTABLE void newContextContent(const QString &skinName, const QString &contentName, QList<int> idList);  // TODO model
    Q_SCRIPTABLE void invalidateContextContent() { emit sendInvalidateContextContent(); }

signals:
    void sendNewContextContent(QString& skinName, QString& contentName, QList<int> idList);
    void sendInvalidateContextContent();

};

#endif // CONTEXTCONTENTRPC_H
