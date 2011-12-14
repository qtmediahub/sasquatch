#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

class RpcConnection;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

private slots:
    void handleConnectButton();
    void handleConnected();
    void handleDisconnected();
    void handleRefreshButton();

private:
    RpcConnection *m_connection;

    QPushButton *m_connectButton;
    QLineEdit *m_connectAddress;
    QSpinBox *m_connectPort;

    QPushButton *m_refreshButton;
    bool m_connected;
};

#endif // MAINWINDOW_H
