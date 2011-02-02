#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHostAddress>

class QTreeView;
class QStackedWidget;
class RemoteControl;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

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
};

#endif // MAINWINDOW_H
