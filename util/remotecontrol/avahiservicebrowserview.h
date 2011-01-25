#ifndef AVAHISERVICEBROWSERVIEW
#define AVAHISERVICEBROWSERVIEW

#include <QTreeView>
#include <QHostAddress>

class QAvahiServiceBrowserModel;

class AvahiServiceBrowserView : public QTreeView
{
    Q_OBJECT
public:
    AvahiServiceBrowserView(QWidget *parent = 0);
    ~AvahiServiceBrowserView();

signals:
    void serviceSelected(const QHostAddress &address, int port);

private slots:
    void handleActivated(const QModelIndex &index);

private:
    QAvahiServiceBrowserModel *m_browserModel;
};

#endif // AVAHISERVICEBROWSERVIEW

