/****************************************************************************

This file is part of the QtMediaHub project on http://www.gitorious.org.

Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation (qt-info@nokia.com)**

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

****************************************************************************/

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
    restart();
    emit rootIndexChanged();
}

QVariant ModelIndexIterator::rootIndex() const
{
    return QVariant::fromValue<QModelIndex>(m_rootIndex);
}

void ModelIndexIterator::restart()
{
    m_state = NotStarted;
    m_queue.clear();
    m_queue.enqueue(m_rootIndex);
    m_currentIndex = QModelIndex();
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
            if (m_queue.isEmpty()) {
                m_state = Done;
                break;
            }
            m_currentIndex = m_model->index(0, 0, m_queue.dequeue());
        } else {
            m_currentIndex = m_currentIndex.sibling(m_currentIndex.row()+1, m_currentIndex.column());
            if (!m_currentIndex.isValid())
                continue;
        }

        if (m_model->hasChildren(m_currentIndex))
            m_queue.enqueue(m_currentIndex);

        if (m_currentIndex.data(role) == m_filterValue)
            break;
    }

    emit dataChanged();
    return m_currentIndex.isValid();
}

Q_DECLARE_METATYPE(QModelIndex)

