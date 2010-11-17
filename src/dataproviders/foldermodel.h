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

#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QtDeclarative>
#include <QStringList>
#include <QUrl>
#include <QAbstractListModel>

class QModelIndex;
class FolderModelPrivate;

class FolderModel : public QAbstractListModel, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)

    Q_PROPERTY(QUrl folder READ folder WRITE setFolder NOTIFY folderChanged)
    Q_PROPERTY(QString folderName READ folderName NOTIFY folderNameChanged)
    Q_PROPERTY(QUrl parentFolder READ parentFolder NOTIFY folderChanged)
    Q_PROPERTY(QStringList folderList READ folderList)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)
    Q_PROPERTY(SortField sortField READ sortField WRITE setSortField)
    Q_PROPERTY(bool sortReversed READ sortReversed WRITE setSortReversed)
    Q_PROPERTY(bool showDirs READ showDirs WRITE setShowDirs)
    Q_PROPERTY(bool showDotAndDotDot READ showDotAndDotDot WRITE setShowDotAndDotDot)
    Q_PROPERTY(bool showOnlyReadable READ showOnlyReadable WRITE setShowOnlyReadable)
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(int lastSelected READ lastSelected WRITE setLastSelected)

public:
    FolderModel(QObject *parent = 0);
    ~FolderModel();

    enum Roles { FilePathRole = Qt::UserRole+1,
                 FileNameRole = Qt::UserRole+2,
                 IconPathRole = Qt::UserRole+3,
                 FileTypeRole = Qt::UserRole+4
               };

    int rowCount(const QModelIndex &parent) const;
    int count() const { return rowCount(QModelIndex()); }
    int lastSelected() const;
    void setLastSelected(int selection);
    QVariant data(const QModelIndex &index, int role) const;
    QUrl folder() const;
    void setFolder(const QUrl &folder);
    QString folderName() const;
    QUrl parentFolder() const;
    QStringList folderList() const;
    QStringList nameFilters() const;
    void setNameFilters(const QStringList &filters);

    enum SortField { Unsorted, Name, Time, Size, Type, DirsFirst, DirsLast, IgnoreCase };
    SortField sortField() const;
    void setSortField(SortField field);
    Q_ENUMS(SortField)

    bool sortReversed() const;
    void setSortReversed(bool rev);

    bool showDirs() const;
    void setShowDirs(bool);
    bool showDotAndDotDot() const;
    void setShowDotAndDotDot(bool);
    bool showOnlyReadable() const;
    void setShowOnlyReadable(bool);

    Q_INVOKABLE bool isFolder(int index) const;

    // for QDeclarativeParserStatus to update the QML parser state
    virtual void classBegin();
    virtual void componentComplete();

signals:
    void folderChanged();
    void folderNameChanged();
    void refreshed();

private slots:
    void refresh();
    void inserted(const QModelIndex &index, int start, int end);
    void removed(const QModelIndex &index, int start, int end);
    void handleDataChanged(const QModelIndex &start, const QModelIndex &end);
    void thumbnailCreated(QModelIndex modelIndex);

private:
    Q_DISABLE_COPY(FolderModel)
    FolderModelPrivate *d;
};

QML_DECLARE_TYPE(FolderModel)

#endif // FOLDERMODEL_H
