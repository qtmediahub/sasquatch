#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHostAddress>

#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN)
#include <QtNetwork>
#endif

class QTreeView;
class QStackedWidget;
class RemoteControl;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:

private slots:
    void showServiceBrowser();
    void handleServiceSelected(const QHostAddress &address, int port);
#ifdef QMH_NO_AVAHI
    void addService();
#endif

private:
    QStackedWidget *m_stackedWidget;
    QTreeView *m_serviceBrowserView;
    RemoteControl *m_remoteControl;
    QAction *m_backAction, *m_optionsAction;
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN)
    QNetworkSession *m_session;
#endif
};

#endif // MAINWINDOW_H
