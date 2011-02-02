#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include <QWidget>
#include "ui_remotecontrol.h"
#include <QHostAddress>

class RpcConnection;

class RemoteControl : public QWidget
{
    Q_OBJECT
public:
    RemoteControl(QWidget *parent = 0);
    ~RemoteControl();

public slots:
    void connectToService(const QHostAddress &address, int port);
    void disconnectFromService();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void sendButtonPress(int action);
    void enableButtons();
    void disableButtons();

private:
    Ui::RemoteControl ui;
    RpcConnection *m_connection;
};

#endif // REMOTECONTROL_H

