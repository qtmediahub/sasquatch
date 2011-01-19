#ifndef RPCAPI_H
#define RPCAPI_H

#include <QObject>
#include <QHash>

class Frontend;

class RpcApi : public QObject
{
    Q_OBJECT
public:
    RpcApi(Frontend *frontend);
    ~RpcApi();

public slots:
    void remoteControlButtonPressed(const QString &button);

private:
    Frontend *m_frontend;
    QHash<QString, int> m_keyMap;
};

#endif // RPCAPI_H

