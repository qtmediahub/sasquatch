#include "staticservicebrowserview.h"
#include <QFile>
#include <QApplication>
#include <QTextStream>
#include <QAction>

StaticServiceBrowserView::StaticServiceBrowserView(QWidget *parent)
    : QTreeView(parent)
{
    QFile file(":/services.conf");
    if (!file.open(QFile::ReadOnly)) {
        qWarning("Failed to open static services file");
        return;
    }

    m_model = new QStandardItemModel(this);
    QStringList headerLabels;
    headerLabels << tr("Host Name") << tr("IP") << tr("Port");
    m_model->setHorizontalHeaderLabels(headerLabels);

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
        columns[0]->setEditable(false);
        columns.append(new QStandardItem);
        columns[1]->setText(ip);
        columns[1]->setEditable(false);
        columns.append(new QStandardItem);
        columns[2]->setText(port);
        columns[2]->setEditable(false);

        rootItem->appendRow(columns);
    }
    
    setModel(m_model);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(handleActivated(QModelIndex)));

    for (int i = 0; i < m_model->columnCount(); i++)
        resizeColumnToContents(i);
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
}

