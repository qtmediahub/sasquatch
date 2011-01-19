#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include <QWidget>
#include "ui_remotecontrol.h"

class RpcConnection;

class RemoteControl : public QWidget
{
    Q_OBJECT
public:
    RemoteControl(QWidget *parent = 0);
    ~RemoteControl();

private slots:
    void sendButtonPress(const QString &button);

private:
    Ui::RemoteControl ui;
    RpcConnection *m_connection;
};

#endif // REMOTECONTROL_H

