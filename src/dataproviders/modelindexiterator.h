#ifndef MODELINDEXITERATOR_H
#define MODELINDEXITERATOR_H

#include <QObject>
#include <QModelIndex>
#include <QtDeclarative>

class ModelIndexIterator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant rootIndex READ rootIndex WRITE setRootIndex NOTIFY rootIndexChanged)
    Q_PROPERTY(QString filterRole READ filterRole WRITE setFilterRole NOTIFY filterRoleChanged)
    Q_PROPERTY(QString filterValue READ filterValue WRITE setFilterValue NOTIFY filterValueChanged)
    Q_PROPERTY(QObject *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QString dataRole READ dataRole WRITE setDataRole NOTIFY dataRoleChanged)
    Q_PROPERTY(QVariant data READ data NOTIFY dataChanged)

public:
    ModelIndexIterator(QObject *parent = 0);
    ~ModelIndexIterator();

    void setRootIndex(const QVariant &index);
    QVariant rootIndex() const;

    void setFilterValue(const QString &value);
    QString filterValue() const;

    void setFilterRole(const QString &role);
    QString filterRole() const;

    void setModel(QObject *model);
    QObject *model() const;

    void setDataRole(const QString &role);
    QString dataRole() const;

    QVariant data() const;

    Q_INVOKABLE bool next();

signals:
    void rootIndexChanged();
    void filterRoleChanged();
    void filterValueChanged();
    void modelChanged();
    void dataRoleChanged();
    void dataChanged();

private:
    int roleFromName(const QString &name) const;

    QAbstractItemModel *m_model;
    QModelIndex m_rootIndex;
    QString m_filterRole;
    QString m_filterValue;
    QString m_dataRole;
    QModelIndex m_currentIndex;
    enum State { NotStarted, Started, Done } m_state;
    QStack<QModelIndex> m_stack;
};

QML_DECLARE_TYPE(ModelIndexIterator)

#endif // MODELINDEXITERATOR_H

