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

#ifndef MODELINDEXITERATOR_H
#define MODELINDEXITERATOR_H

#include <QObject>
#include <QModelIndex>
#include <QtDeclarative>

class ModelIndexIterator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant rootIndex READ rootIndex WRITE setRootIndex NOTIFY rootIndexChanged)
    Q_PROPERTY(int fromRow READ fromRow WRITE setFromRow NOTIFY fromRowChanged)
    Q_PROPERTY(QString childFilterRole READ childFilterRole WRITE setChildFilterRole NOTIFY childFilterRoleChanged)
    Q_PROPERTY(QString childFilterValue READ childFilterValue WRITE setChildFilterValue NOTIFY childFilterValueChanged)
    Q_PROPERTY(QString parentFilterRole READ parentFilterRole WRITE setParentFilterRole NOTIFY parentFilterRoleChanged)
    Q_PROPERTY(QString parentFilterValue READ parentFilterValue WRITE setParentFilterValue NOTIFY parentFilterValueChanged)
    Q_PROPERTY(QObject *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QString dataRole READ dataRole WRITE setDataRole NOTIFY dataRoleChanged)
    Q_PROPERTY(QVariant data READ data NOTIFY dataChanged)

public:
    ModelIndexIterator(QObject *parent = 0);
    ~ModelIndexIterator();

    void setRootIndex(const QVariant &index);
    QVariant rootIndex() const;

    void setFromRow(int row);
    int fromRow() const;

    void setChildFilterValue(const QString &value);
    QString childFilterValue() const;

    void setChildFilterRole(const QString &role);
    QString childFilterRole() const;

    void setParentFilterValue(const QString &value);
    QString parentFilterValue() const;

    void setParentFilterRole(const QString &role);
    QString parentFilterRole() const;

    void setModel(QObject *model);
    QObject *model() const;

    void setDataRole(const QString &role);
    QString dataRole() const;

    QVariant data() const;

    Q_INVOKABLE bool previous();
    Q_INVOKABLE bool next();
    Q_INVOKABLE void restart();

signals:
    void rootIndexChanged();
    void fromRowChanged();
    void childFilterRoleChanged();
    void childFilterValueChanged();
    void parentFilterRoleChanged();
    void parentFilterValueChanged();
    void modelChanged();
    void dataRoleChanged();
    void dataChanged();

private:
    int roleFromName(const QString &name) const;

    QAbstractItemModel *m_model;
    QModelIndex m_rootIndex;
    int m_fromRow;
    QString m_childFilterRole;
    QString m_childFilterValue;
    QString m_parentFilterRole;
    QString m_parentFilterValue;
    QString m_dataRole;
    QModelIndex m_currentIndex;
    enum State { NotStarted, Started, Done } m_state;
    QList<QModelIndex> m_queue;
};

QML_DECLARE_TYPE(ModelIndexIterator)

#endif // MODELINDEXITERATOR_H

