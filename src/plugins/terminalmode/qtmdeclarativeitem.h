#ifndef QTMDECLARATIVEITEM_H
#define QTMDECLARATIVEITEM_H

#include <QDeclarativeItem>

#include "QSignalMapper"

#include "QTmVncClient.h"
#include "QTmUpnpControlPoint.h"
#include "QTmNetworkObserver.h"
#include "QTmClientProfile.h"
#include "QTmClientRules.h"
#include "QTmClient.h"


class QTmDeclarativeItem : public QDeclarativeItem
{
    Q_OBJECT
public:
    static void initModule();

    explicit QTmDeclarativeItem(QDeclarativeItem *parent = 0);

private slots:

    void upnpApplicationAdded(QTmApplication *application);
    void upnpApplicationStarted(QTmApplication *application);

    void deviceDetected(QString interfaceName, QHostAddress interface);
    void deviceLost(QHostAddress interface);

private:

    QGraphicsProxyWidget *mPW;
    QTmClient *m_TerminalMode;
    int mVNCid;

    bool mIsConnected;
};

#endif // QTMDECLARATIVEITEM_H
