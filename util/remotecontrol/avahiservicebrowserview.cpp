#include "avahiservicebrowserview.h"
#include "qavahiservicebrowsermodel.h"

AvahiServiceBrowserView::AvahiServiceBrowserView(QWidget *parent)
    : QTreeView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    m_browserModel = new QAvahiServiceBrowserModel(this);
    m_browserModel->browse("_qmh._tcp");
    setModel(m_browserModel);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(handleActivated(QModelIndex)));

    for (int i = 0; i < m_browserModel->columnCount(); i++)
        resizeColumnToContents(i);
    selectionModel()->select(rootItem->child(0)->index(), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

AvahiServiceBrowserView::~AvahiServiceBrowserView()
{
}

void AvahiServiceBrowserView::handleActivated(const QModelIndex &index)
{
    QAvahiServiceBrowserModel::Service service = m_browserModel->serviceFromIndex(index);
    emit serviceSelected(service.address, service.port);
}
