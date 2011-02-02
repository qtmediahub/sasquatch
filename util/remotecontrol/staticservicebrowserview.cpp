#include "staticservicebrowserview.h"
#include <QFile>
#include <QApplication>
#include <QTextStream>
#include <QAction>
#include <QMenu>

StaticServiceBrowserView::StaticServiceBrowserView(QWidget *parent)
    : QTreeView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);

    m_model = new QStandardItemModel(this);
    initModelFromFile(":/services.conf");
    setModel(m_model);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(handleActivated(QModelIndex)));

    for (int i = 0; i < m_model->columnCount(); i++)
        resizeColumnToContents(i);

    selectionModel()->select(m_model->index(0, 0), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

StaticServiceBrowserView::~StaticServiceBrowserView()
{
}

void StaticServiceBrowserView::initModelFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("Failed to open static services file");
        return;
    }

    QStringList headerLabels;
    headerLabels << StaticServiceBrowserView::tr("Host Name")
                 << StaticServiceBrowserView::tr("IP")
                 << StaticServiceBrowserView::tr("Port");
    m_model->setHorizontalHeaderLabels(headerLabels);

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

        addService(hostName, ip, port);
    }
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

void StaticServiceBrowserView::addService(const QString &hostName, const QString &ip, const QString &port)
{
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

    m_model->appendRow(columns);
}

void StaticServiceBrowserView::removeService()
{
    m_model->removeRow(currentIndex().row());
}
