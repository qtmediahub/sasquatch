#ifndef STATICSERVICEBROWSERVIEW
#define STATICSERVICEBROWSERVIEW

#include <QTreeView>
#include <QHostAddress>
#include <QStandardItemModel>

class StaticServiceBrowserView : public QTreeView
{
    Q_OBJECT
public:
    StaticServiceBrowserView(QWidget *parent = 0);
    ~StaticServiceBrowserView();

signals:
    void serviceSelected(const QHostAddress &address, int port);

private slots:
    void handleActivated(const QModelIndex &index);

private:
    QStandardItemModel *m_model;
};

#endif // STATICSERVICEBROWSERVIEW

