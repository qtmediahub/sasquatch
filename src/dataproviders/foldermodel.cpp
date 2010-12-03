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

#include "foldermodel.h"

#include <QDirModel>
#include <QDebug>

#include "thumbnailer.h"

int TypeID0 = qRegisterMetaType<QModelIndex>("QModelIndex");

class FolderModelPrivate
{
public:
    FolderModelPrivate()
        : sortField(FolderModel::Name)
        , sortReversed(false)
        , count(0)
    {
        nameFilters << QLatin1String("*");
    }

    void updateSorting()
    {
        QDir::SortFlags flags = 0;
        switch(sortField) {
        case FolderModel::Unsorted:
            flags |= QDir::Unsorted;
            break;
        case FolderModel::Name:
            flags |= QDir::Name;
            break;
        case FolderModel::Time:
            flags |= QDir::Time;
            break;
        case FolderModel::Size:
            flags |= QDir::Size;
            break;
        case FolderModel::Type:
            flags |= QDir::Type;
            break;
        case FolderModel::DirsFirst:
            flags |= QDir::DirsFirst;
            break;
        case FolderModel::DirsLast:
            flags |= QDir::DirsLast;
            break;
        case FolderModel::IgnoreCase:
            flags |= QDir::IgnoreCase;
            break;
        }

        if (sortReversed)
            flags |= QDir::Reversed;

        model.setSorting(flags);
    }

    QDirModel model;
    QUrl folder;
    QStringList nameFilters;
    QModelIndex folderIndex;
    FolderModel::SortField sortField;
    bool sortReversed;
    int count;
};

FolderModel::FolderModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[FilePathRole] = "filePath";
    roles[FileNameRole] = "fileName";
    roles[IconPathRole] = "iconPath";
    roles[FileTypeRole] = "fileType";
    setRoleNames(roles);

    d = new FolderModelPrivate;
    d->model.setFilter(QDir::AllDirs | QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    connect(&d->model, SIGNAL(rowsInserted(const QModelIndex&,int,int))
            , this, SLOT(inserted(const QModelIndex&,int,int)));
    connect(&d->model, SIGNAL(rowsRemoved(const QModelIndex&,int,int))
            , this, SLOT(removed(const QModelIndex&,int,int)));
    connect(&d->model, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&))
            , this, SLOT(handleDataChanged(const QModelIndex&,const QModelIndex&)));
    connect(&d->model, SIGNAL(modelReset()), this, SLOT(refresh()));
    connect(&d->model, SIGNAL(layoutChanged()), this, SLOT(refresh()));

    connect(Thumbnailer::getInstance(), SIGNAL(creationFinished(QModelIndex))
            , this, SLOT(thumbnailCreated(QModelIndex)));
}

FolderModel::~FolderModel()
{
    delete d;
}

int FolderModel::lastSelected() const
{
    // TODO
    return 0;
}

void FolderModel::setLastSelected(int selection)
{
    // TODO
    Q_UNUSED(selection)
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
    QVariant rv;
    QModelIndex modelIndex = d->model.index(index.row(), 0, d->folderIndex);
    if (modelIndex.isValid()) {
        if (role == FilePathRole)
            rv = d->model.data(modelIndex, QDirModel::FilePathRole).toString();
        else if (role == FileNameRole || role == Qt::DisplayRole)
            rv = QFileInfo(d->model.data(modelIndex, QDirModel::FileNameRole).toString()).baseName();
        else if (role == IconPathRole)
            rv = Thumbnailer::getInstance()->thumbnail(d->model.data(modelIndex, QDirModel::FilePathRole).toString(), modelIndex);
        else if (role == FileTypeRole)
            rv = QFileInfo(d->model.data(modelIndex, QDirModel::FilePathRole).toString()).isDir();
    }

    return rv;
}

int FolderModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->count;
}

QUrl FolderModel::folder() const
{
    QString localFile = d->folder.toLocalFile();

    // ensure to have a slash at the end
    if (!localFile.endsWith('/'))
        localFile.append('/');

    return QUrl::fromLocalFile(localFile);
}

void FolderModel::setFolder(const QUrl &folder)
{
    if (folder == d->folder)
        return;

    QModelIndex index = d->model.index(folder.toLocalFile());
    if ((index.isValid() && d->model.isDir(index)) || folder.toLocalFile().isEmpty()) {
        d->folder = folder;

        QMetaObject::invokeMethod(this, "refresh", Qt::QueuedConnection);
        emit folderChanged();
        emit folderNameChanged();
    }
}

QString FolderModel::folderName() const
{
    QString ret = d->folder.toString(QUrl::StripTrailingSlash);
    int i = ret.lastIndexOf('/')+1;
    return ret.mid(i,ret.length()-i);
}

QUrl FolderModel::parentFolder() const
{
    QString localFile = d->folder.toLocalFile();

    if (!localFile.isEmpty()) {
        QDir dir(localFile);
        dir.cdUp();
        localFile = dir.path();
    } else {
        int pos = d->folder.path().lastIndexOf(QLatin1Char('/'));
        if (pos == -1)
            return QUrl();
        localFile = d->folder.path().left(pos);
    }
    // ensure to have a slash at the end
    if (!localFile.endsWith('/'))
        localFile.append('/');

    return QUrl::fromLocalFile(localFile);
}

QStringList FolderModel::folderList() const
{
    QStringList list;
    int i = 0;

    QModelIndex idx = d->model.index(i,0, d->folderIndex);
    while (idx.isValid()) {
        list << d->model.data(idx, QDirModel::FilePathRole).toString();
        idx = d->model.index(++i,0, d->folderIndex);
    }

    return list;
}

QStringList FolderModel::nameFilters() const
{
    return d->nameFilters;
}

void FolderModel::setNameFilters(const QStringList &filters)
{
    d->nameFilters = filters;
    d->model.setNameFilters(d->nameFilters);
}

void FolderModel::classBegin()
{
}

void FolderModel::componentComplete()
{
    if (!d->folder.isValid() || !QDir().exists(d->folder.toLocalFile()))
        setFolder(QUrl(QLatin1String("file://")+QDir::currentPath()));

    if (!d->folderIndex.isValid())
        QMetaObject::invokeMethod(this, "refresh", Qt::QueuedConnection);
}

FolderModel::SortField FolderModel::sortField() const
{
    return d->sortField;
}

void FolderModel::setSortField(SortField field)
{
    if (field != d->sortField) {
        d->sortField = field;
        d->updateSorting();
    }
}

bool FolderModel::sortReversed() const
{
    return d->sortReversed;
}

void FolderModel::setSortReversed(bool rev)
{
    if (rev != d->sortReversed) {
        d->sortReversed = rev;
        d->updateSorting();
    }
}

bool FolderModel::isFolder(int index) const
{
    if (index != -1) {
        QModelIndex idx = d->model.index(index, 0, d->folderIndex);
        if (idx.isValid())
            return d->model.isDir(idx);
    }
    return false;
}

void FolderModel::refresh()
{
    d->folderIndex = QModelIndex();
    if (d->count) {
        emit beginRemoveRows(QModelIndex(), 0, d->count);
        d->count = 0;
        emit endRemoveRows();
    }
    d->folderIndex = d->model.index(d->folder.toLocalFile());
    int newcount = d->model.rowCount(d->folderIndex);
    if (newcount) {
        emit beginInsertRows(QModelIndex(), 0, newcount-1);
        d->count = newcount;
        emit endInsertRows();
    }

    emit refreshed();
}

void FolderModel::inserted(const QModelIndex &index, int start, int end)
{
    if (index == d->folderIndex) {
        emit beginInsertRows(QModelIndex(), start, end);
        d->count = d->model.rowCount(d->folderIndex);
        emit endInsertRows();
    }
}

void FolderModel::removed(const QModelIndex &index, int start, int end)
{
    if (index == d->folderIndex) {
        emit beginRemoveRows(QModelIndex(), start, end);
        d->count = d->model.rowCount(d->folderIndex);
        emit endRemoveRows();
    }
}

void FolderModel::handleDataChanged(const QModelIndex &start, const QModelIndex &end)
{
    if (start.parent() == d->folderIndex) {

        emit dataChanged(index(start.row(),0), index(end.row(),0));
    }
}

void FolderModel::thumbnailCreated(QModelIndex modelIndex)
{
    emit dataChanged(index(modelIndex.row(),0), index(modelIndex.row(),0));
}

bool FolderModel::showDirs() const
{
    return d->model.filter() & QDir::AllDirs;
}

void  FolderModel::setShowDirs(bool on)
{
    if (!(d->model.filter() & QDir::AllDirs) == !on)
        return;
    if (on)
        d->model.setFilter(d->model.filter() | QDir::AllDirs | QDir::Drives);
    else
        d->model.setFilter(d->model.filter() & ~(QDir::AllDirs | QDir::Drives));
}

bool FolderModel::showDotAndDotDot() const
{
    return !(d->model.filter() & QDir::NoDotAndDotDot);
}

void  FolderModel::setShowDotAndDotDot(bool on)
{
    if (!(d->model.filter() & QDir::NoDotAndDotDot) == on)
        return;
    if (on)
        d->model.setFilter(d->model.filter() & ~QDir::NoDotAndDotDot);
    else
        d->model.setFilter(d->model.filter() | QDir::NoDotAndDotDot);
}

bool FolderModel::showOnlyReadable() const
{
    return d->model.filter() & QDir::Readable;
}

void FolderModel::setShowOnlyReadable(bool on)
{
    if (!(d->model.filter() & QDir::Readable) == !on)
        return;
    if (on)
        d->model.setFilter(d->model.filter() | QDir::Readable);
    else
        d->model.setFilter(d->model.filter() & ~QDir::Readable);
}

