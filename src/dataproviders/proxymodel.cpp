/****************************************************************************
**
** Copyright (c) 2010 Girish Ramakrishnan <girish@forwardbias.in>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#include "proxymodel.h"
#include <QtGui>

ProxyModel::ProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
{
}

ProxyModel::~ProxyModel()
{
}

QModelIndex ProxyModel::index(int row, int col, const QModelIndex &idx) const
{
    if (idx.isValid())
        return QModelIndex(); // only list models are supported

    return createIndex(row, col);
}

QModelIndex ProxyModel::parent(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return QModelIndex(); // only list models are supported
}

int ProxyModel::rowCount(const QModelIndex &idx) const
{
    if (idx.isValid())
        return 0;
    const int sourceRowCount = sourceModel() ? sourceModel()->rowCount(idx) : 0;
    return sourceRowCount + m_items.count();
}

int ProxyModel::columnCount(const QModelIndex &idx) const
{
    return sourceModel() ? sourceModel()->columnCount(idx) : 0;
}

QVariant ProxyModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();
    const int sourceRowCount = sourceModel() ? sourceModel()->rowCount() : 0;
    if (idx.row() >= sourceRowCount)
        return m_items[idx.row()-sourceRowCount]->data(role);
    else if (sourceModel())
        return sourceModel()->data(sourceModel()->index(idx.row(), idx.column()), role);
    else
        return QVariant();
}

bool ProxyModel::hasChildren(const QModelIndex &parent) const
{
    return !parent.isValid();
}

Qt::ItemFlags ProxyModel::flags(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return Qt::ItemFlags();
    const int sourceRowCount = sourceModel() ? sourceModel()->rowCount() : 0;
    if (idx.row() >= sourceRowCount)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable; // ## make this settable per row
    else if (sourceModel())
        return sourceModel()->flags(sourceModel()->index(idx.row(), idx.column()));
    else
        return Qt::ItemFlags();
}

QModelIndex ProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid())
        return QModelIndex();
    else if (proxyIndex.row() >= sourceModel()->rowCount())
        return QModelIndex(); // this is an index in the proxy
    else
        return sourceModel()->index(proxyIndex.row(), proxyIndex.column());
}

QModelIndex ProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
        return QModelIndex();
    else
        return createIndex(sourceIndex.row(), sourceIndex.column());
}

int ProxyModel::appendRow(int role, const QVariant &data)
{
    QHash<int, QVariant> hash;
    hash[role] = data;
    return appendRow(hash);
}

int ProxyModel::appendRow(const QHash<int, QVariant> &data)
{
    const int sourceRowCount = sourceModel() ? sourceModel()->rowCount() : 0;
    beginInsertRows(QModelIndex(), sourceRowCount + m_items.count(), sourceRowCount + m_items.count());
    ProxyModelItem *item = new ProxyModelItem(this);
    item->setData(data);
    m_items.append(item);
    endInsertRows();
    return sourceRowCount + m_items.count() - 1;
}

int ProxyModel::appendRow(const QScriptValue &value)
{
    QScriptValueIterator it(value);
    QHash<int, QVariant> hash;
    while (it.hasNext()) {
        it.next();
        if (it.name() == "display") {
            hash.insert(Qt::DisplayRole, it.value().toString());
        }
    }
    return appendRow(hash);
}

void ProxyModel::setSourceModel(QAbstractItemModel *model)
{
   // destroyed and reset is already connected by QAbstractProxyModel
    disconnect(sourceModel(), 0);
    QAbstractProxyModel::setSourceModel(model);
    connect(model, SIGNAL(destroyed()), this, SLOT(reset()));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(reset()));
    connect(model, SIGNAL(modelReset()), this, SLOT(reset()));
    connect(model, SIGNAL(layoutChanged()), this, SLOT(reset()));
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(reset()));
    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(reset()));

    // ## what we don't handle
    // headerDataChanged(Qt::Orientation,int,int))
    // columnsRemoved(QModelIndex,int,int)
    // columnsInserted(QModelIndex,int,int))
    // rowsAboutToBeRemoved(QModelIndex,int,int))
}

void ProxyModel::setSource(const QVariant &model)
{
    if (model.type() == QMetaType::QStringList) {
        setSourceModel(new QStringListModel(model.toStringList(), this));
    } else if (model.type() == QMetaType::QVariantList) {
        // ## Lots of assumptions here
        QVariantList list = model.toList();
        QStringList strings;
        foreach(const QVariant &item, list) {
            strings.append(item.toString());
        }
        setSourceModel(new QStringListModel(strings, this));
    } else if (model.type() == QMetaType::QObjectStar) {
        QAbstractItemModel *aim = qobject_cast<QAbstractItemModel *>(model.value<QObject*>());
        if (aim)
            setSourceModel(aim);
    } else {
        qDebug() << model.type() << " is not a supported model type in ProxyModel";
    }
    m_modelVariant = model;
    emit sourceChanged();
}

QVariant ProxyModel::source() const
{
    return m_modelVariant;
}

void ProxyModel::append_item(QDeclarativeListProperty<ProxyModelItem> *list, ProxyModelItem *item)
{
    ProxyModel *proxyModel = static_cast<ProxyModel *>(list->object);
    proxyModel->m_items.append(item);
}

void ProxyModel::clear_list(QDeclarativeListProperty<ProxyModelItem> *list)
{
    ProxyModel *proxyModel = static_cast<ProxyModel *>(list->object);
    qDeleteAll(proxyModel->m_items);
    proxyModel->m_items.clear();
}

QDeclarativeListProperty<ProxyModelItem> ProxyModel::items()
{
    QDeclarativeListProperty<ProxyModelItem> list(this, m_items);
    list.append = &ProxyModel::append_item;
    list.clear = &ProxyModel::clear_list;
    return list;
}

