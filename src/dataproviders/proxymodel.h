/****************************************************************************
**
** Copyright (c) 2010 Girish Ramakrishnan <girish@forwardbias.in>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#ifndef PROXYMODEL_H
#define PROXYMODEL_H

#include <QtDeclarative>
#include <QAbstractProxyModel>

class ProxyModelItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString display READ display WRITE setDisplay NOTIFY displayChanged)
public:
    ProxyModelItem(QObject *parent = 0) : QObject(parent) { }
    ~ProxyModelItem() { }

    QString display() const { return m_data.value(Qt::DisplayRole).toString(); }
    void setDisplay(const QString &display) { m_data[Qt::DisplayRole] = display; emit displayChanged(); }

    QVariant data(int role) const { return m_data.value(role); }
    void setData(int role, const QVariant &var) { m_data[role] = var; }
    void setData(const QHash<int, QVariant> &hash) { m_data = hash; }

signals:
    void displayChanged();

private:
    QHash<int, QVariant> m_data;
};

class ProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QVariant source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QDeclarativeListProperty<ProxyModelItem> items READ items)
    Q_CLASSINFO("DefaultProperty", "items")

public:
    ProxyModel(QObject *parent = 0);
    ~ProxyModel();

    // reimp from QAIM
    QModelIndex index(int row, int col, const QModelIndex &idx) const;
    QModelIndex parent(const QModelIndex &idx) const;
    int rowCount(const QModelIndex &idx) const;
    int columnCount(const QModelIndex &idx) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    // reimp from QAbstractProxyModel
    void setSourceModel(QAbstractItemModel *source);
    QModelIndex mapToSource(const QModelIndex &idx) const;
    QModelIndex mapFromSource(const QModelIndex &idx) const;

    int appendRow(int role, const QVariant &text);
    int appendRow(const QHash<int, QVariant> &data);

    // qml
    void setSource(const QVariant &model);
    QVariant source() const;
    Q_INVOKABLE int appendRow(const QScriptValue &value);
    QDeclarativeListProperty<ProxyModelItem> items();

public slots:
    void reset() { QAbstractProxyModel::reset(); }

signals:
    void sourceChanged();

private:
    static void append_item(QDeclarativeListProperty<ProxyModelItem> *list, ProxyModelItem *item);
    static void clear_list(QDeclarativeListProperty<ProxyModelItem> *list);

    QVariant m_modelVariant;
    QList<ProxyModelItem *> m_items;
};

QML_DECLARE_TYPE(ProxyModel)

#endif // PROXYMODEL_H

