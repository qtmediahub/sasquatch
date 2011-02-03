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

public slots:
    void addService(const QString &hostName, const QString &ip, const QString &port);
    void removeService();
    void save();

signals:
    void serviceSelected(const QHostAddress &address, int port);

private slots:
    void handleActivated(const QModelIndex &index);

private:
    void initModelFromFile(const QString &fileName);
    QStandardItemModel *m_model;
};

#endif // STATICSERVICEBROWSERVIEW

