#include "staticservicebrowserview.h"
#include <QFile>
#include <QTextStream>

StaticServiceBrowserView::StaticServiceBrowserView(QWidget *parent)
    : QTreeView(parent)
{
    QFile file(":/services.conf");
    if (!file.open(QFile::ReadOnly)) {
        qWarning("Failed to open static services file");
        return;
    }

    m_model = new QStandardItemModel(this);
    QStandardItem *rootItem = m_model->invisibleRootItem();

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString hostName;
        stream >> hostName;
        if (hostName.startsWith('#')) {
            stream.readLine();
            continue;
        }
        QString ip, port;
        stream >> ip >> port;

        QList<QStandardItem *> columns;
        columns.append(new QStandardItem);
        columns[0]->setText(hostName);
        columns.append(new QStandardItem);
        columns[1]->setText(ip);
        columns.append(new QStandardItem);
        columns[2]->setText(port);

        rootItem->appendRow(columns);
    }
    
    setModel(m_model);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(handleActivated(QModelIndex)));
}

StaticServiceBrowserView::~StaticServiceBrowserView()
{
}

void StaticServiceBrowserView::handleActivated(const QModelIndex &index)
{
    QStandardItem *item = m_model->invisibleRootItem();
    QString hostName = item->child(index.row(), 0)->text();
    Q_UNUSED(hostName);
    QString ip = item->child(index.row(), 1)->text();
    QString port = item->child(index.row(), 2)->text();

    emit serviceSelected(QHostAddress(ip), port.toInt());
    hide();
}

