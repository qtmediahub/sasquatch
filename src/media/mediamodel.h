/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Nokia Corporation donald.carr@nokia.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include <QtCore>
#include <QtGui>
#include <QtDeclarative>
#include <QtSql>

class QThread;
class MediaScanner;
class DbReader;
class GlobalSettings;

class MediaModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString structure READ structure WRITE setStructure NOTIFY structureChanged)
    Q_PROPERTY(QString mediaType READ mediaType WRITE setMediaType NOTIFY mediaTypeChanged)
    Q_PROPERTY(QString sqlCondition READ sqlCondition WRITE setSqlCondition NOTIFY sqlConditionChanged)
    Q_PROPERTY(QString part READ part NOTIFY partChanged)
    Q_PROPERTY(DotDotPosition dotDotPosition READ dotDotPosition WRITE setDotDotPosition NOTIFY dotDotPositionChanged)

    Q_ENUMS(DotDotPosition)

public:
    enum CustomRole {
        DotDotRole = Qt::UserRole,
        MediaTypeRole,
        IsLeafRole,
        ModelIndexRole,
        PreviewUrlRole,
        FieldRolesBegin
    };

    enum DotDotPosition {
        Beginning,
        End,
        Nowhere
    };

    MediaModel(QObject *parent = 0);
    ~MediaModel();

    QString structure() const;
    void setStructure(const QString &str);

    QString mediaType() const;
    void setMediaType(const QString &type);

    QString sqlCondition() const;
    void setSqlCondition(const QString &sqlCondition);

    DotDotPosition dotDotPosition() const;
    void setDotDotPosition(DotDotPosition position);

    Q_INVOKABLE void back(int count = 1);
    Q_INVOKABLE void enter(int index);

    Q_INVOKABLE QList<int> getIdList() const;
    Q_INVOKABLE int indexById(int id) const;


    QString part() const;

    // reimp
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int col, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &idx) const;
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &idx = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);
    QMap<int, QVariant> itemData(const QModelIndex &index) const;

    // PathView hack
    Q_INVOKABLE void reset() { QAbstractItemModel::reset(); }

    bool isLeafLevel() const;

    static QHash<int, QByteArray> roleToNameMapping();
    static QHash<QString, int> nameToRoleMapping();
    static QMap<int, QVariant> dynamicRolesDataFromRecord(const QSqlRecord &record);

    // Called by the MediaScanner
    static void createStaticRoleNameMapping();
    static void createDynamicRoleNameMapping(const QString &tableName);

    QSqlQuery leafNodesQuery(int row) const;

signals:
    void structureChanged();
    void mediaTypeChanged();
    void sqlConditionChanged();
    void partChanged();
    void dotDotPositionChanged();

private slots:
    void handleDataReady(DbReader *reader, const QList<QSqlRecord> &data, void *node);
    void handleScanStarted(const QString &type);
    void handleScanFinished(const QString &type);
    void refresh();

private:
    void createNewDbReader();
    void reload();
    QPair<QString, QStringList> buildQuery(const QList<QMap<int, QVariant> > &cursor, bool forceLastPart) const;
    int compareData(int idx, const QSqlRecord &record) const;

    void insertAll(const QList<QSqlRecord> &records);
    void update(const QList<QSqlRecord> &records);

    QMap<int, QVariant> dataFromRecord(const QSqlRecord &record) const;
    QString displayStringFromRecord(const QSqlRecord &record) const;

    GlobalSettings *m_settings;
    QString m_structure;
    QString m_sqlCondition;
    QList<QStringList> m_layoutInfo;
    QList<QMap<int, QVariant> > m_data;
    bool m_loading, m_loaded;

    QList<QMap<int, QVariant> > m_cursor;

    QTimer m_refreshTimer;

    DbReader *m_reader;
    QThread *m_readerThread;
    QString m_mediaType;

    bool m_autoForward;

    DotDotPosition m_dotDotPosition;

    static int s_currentDynamicRole;
    static QHash<int, QByteArray> s_roleToName;
    static QHash<QString, int> s_nameToRole;
};

#endif // MEDIAMODEL_H

