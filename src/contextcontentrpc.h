#ifndef CONTEXTCONTENTRPC_H
#define CONTEXTCONTENTRPC_H

#include <QObject>
#include <QDebug>

class ContextContentRpc : public QObject
{
    Q_OBJECT
public:
    explicit ContextContentRpc(QObject *parent = 0);

    Q_SCRIPTABLE void newContextContent(const QString &skinName, const QString &contentName, QList<int> idList);  // TODO model
    Q_SCRIPTABLE void invalidateContextContent() { emit sendInvalidateContextContent(); }

public slots:
    void selectItemById(qlonglong id) { emit itemSelectedById(id); }

signals:
    void sendNewContextContent(QString& skinName, QString& contentName, QList<int> idList);
    void sendInvalidateContextContent();
    void itemSelectedById(qlonglong id);


};

#endif // CONTEXTCONTENTRPC_H
