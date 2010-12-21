#include "modelindexiterator.h"

ModelIndexIterator::ModelIndexIterator(QObject *parent)
    : QObject(parent), m_model(0), m_state(NotStarted)
{
}

ModelIndexIterator::~ModelIndexIterator()
{
}

void ModelIndexIterator::setRootIndex(const QVariant &index)
{
    QModelIndex rootIndex = qvariant_cast<QModelIndex>(index);
    if (rootIndex == m_rootIndex)
        return;
    m_rootIndex = rootIndex;
    m_state = NotStarted;
    m_stack.clear();
    m_stack.push(m_rootIndex);
    emit rootIndexChanged();
}

QVariant ModelIndexIterator::rootIndex() const
{
    return QVariant::fromValue<QModelIndex>(m_rootIndex);
}

void ModelIndexIterator::setFilterRole(const QString &role)
{
    if (m_filterRole == role)
        return;
    m_filterRole = role;
    emit filterRoleChanged();
}

QString ModelIndexIterator::filterRole() const
{
    return m_filterRole;
}

void ModelIndexIterator::setFilterValue(const QString &value)
{
    if (m_filterValue == value)
        return;
    m_filterValue = value;
    emit filterValueChanged();
}

QString ModelIndexIterator::filterValue() const
{
    return m_filterValue;
}

void ModelIndexIterator::setModel(QObject *model)
{
    if (m_model == model)
        return;
    m_model = qobject_cast<QAbstractItemModel *>(model);
    emit modelChanged();
}

QObject *ModelIndexIterator::model() const
{
    return m_model;
}

void ModelIndexIterator::setDataRole(const QString &role)
{
    if (m_dataRole == role)
        return;
    m_dataRole = role;
    emit dataRoleChanged();
}

QString ModelIndexIterator::dataRole() const
{
    return m_dataRole;
}

int ModelIndexIterator::roleFromName(const QString &roleName) const
{
    if (!m_model)
        return -1;
    const QHash<int, QByteArray> &roleNames = m_model->roleNames();
    QHash<int, QByteArray>::const_iterator it = roleNames.constBegin();
    while (it != roleNames.constEnd()) {
        if (it.value() == roleName.toLatin1())
            return it.key();
        ++it;
    }
    return -1;
}

QVariant ModelIndexIterator::data() const
{
    return m_currentIndex.data(roleFromName(m_dataRole));
}

bool ModelIndexIterator::next()
{
    if (!m_model || m_state == Done)
        return false;

    int role = roleFromName(m_filterRole);

    while (true) {
        if (!m_currentIndex.isValid()) {
            if (m_stack.isEmpty()) {
                m_state = Done;
                break;
            }
            m_currentIndex = m_stack.pop().child(0, 0);
        } else {
            m_currentIndex = m_currentIndex.sibling(m_currentIndex.row()+1, m_currentIndex.column());
            if (!m_currentIndex.isValid())
                continue;
        }

        if (m_model->hasChildren(m_currentIndex)) {
            m_stack.push(m_currentIndex);
        }
        if (m_currentIndex.data(role) == m_filterValue)
            break;
    }

    emit dataChanged();
    return m_currentIndex.isValid();
}

Q_DECLARE_METATYPE(QModelIndex)

