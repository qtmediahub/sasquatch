/****************************************************************************
 *
 * This file is part of the QtMediaHub project on http://www.gitorious.org.
 *
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
 * All rights reserved.
 *
 * Contact:  Nokia Corporation (qt-info@nokia.com)**
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 *
 * ****************************************************************************/

#include "mediamodel.h"
#include <QDirIterator>
#include <QThreadPool>
#include <QTimer>
#include <QtDebug>
#include <QMetaEnum>
#include <QDeclarativeEngine>

MediaModel::MediaModel(MediaModel::MediaType type, QObject *parent)
    : QAbstractItemModel(parent),
      m_type(type),
      m_thread(0),
      m_nowSearching(-1)
{
    qRegisterMetaType<MediaInfo>("MediaInfo");
    qRegisterMetaType<MediaInfo *>("MediaInfo *");

    QHash<int, QByteArray> roleNames = QAbstractItemModel::roleNames();
    roleNames[Qt::DisplayRole] = "display";
    roleNames[DecorationUrlRole] = "decorationUrl";
    roleNames[FilePathRole] = "filePath";
    roleNames[FileNameRole] = "fileName";
    setRoleNames(roleNames);

    Data *data = new Data(QString("/AddNewSource"), tr("Add new source"));
    m_data.append(data);
}

MediaModel::~MediaModel()
{
    for (int i = 0; i < m_data.count(); i++)
        qDeleteAll(m_data[i]->mediaInfos);
    qDeleteAll(m_data);

    // FIXME: Wait until thread is dead
    delete m_thread;
}

void MediaModel::startSearchThread()
{
    if (m_nowSearching != -1)
        return; // already searching some directory

    int i;
    for (i = 0; i < m_data.count()-1; i++) { // leave out the last item
        if (m_data[i]->status == Data::NotSearched)
            break;
    }
    if (i == m_data.count()-1) {
        m_nowSearching = -1;
        return; // all searched
    }

    Q_ASSERT(!m_thread);
    m_thread = new MediaModelThread(this, i, m_data[i]->searchPath);
    m_thread->setAutoDelete(false);
    m_data[i]->status = Data::Searching;
    m_nowSearching = i;
    connect(m_thread, SIGNAL(mediaFound(int, MediaInfo *)), this, SLOT(addMedia(int, MediaInfo *)));
    connect(m_thread, SIGNAL(finished()), this, SLOT(searchThreadFinished()));
    QThreadPool::globalInstance()->start(m_thread);
}

void MediaModel::searchThreadFinished()
{
    Q_ASSERT(m_nowSearching != -1);
    Q_ASSERT(m_thread);
    m_data[m_nowSearching]->status = Data::Searched;
    m_nowSearching = -1;
    delete m_thread;
    m_thread = 0;

    startSearchThread();
}

void MediaModel::stopSearchThread()
{
    m_thread->stop();
}

void MediaModel::addSearchPath(const QString &path, const QString &name)
{
    beginInsertRows(QModelIndex(), m_data.count()-1, m_data.count()-1);
    Data *data = new Data(path, name);
    MediaInfo *mi = new MediaInfo;
    mi->fileName = tr("..");
    mi->filePath = "/DotDot";
    data->mediaInfos.append(mi);
    m_data.insert(m_data.count()-1, data);
    endInsertRows();
    m_frontCovers.insert(path, QImage(m_themePath + "/media/DefaultHardDisk.png"));

    startSearchThread();
}

QModelIndex MediaModel::index(int row, int col, const QModelIndex &parent) const
{
    if (col != 0 || row < 0)
        return QModelIndex();
    if (!parent.isValid()) { // top level
        if (row >= m_data.count())
            return QModelIndex();
        return createIndex(row, 0, 0);
    } else { // first level
        Data *data = m_data.value(parent.row());
        if (!data || row >= data->mediaInfos.count())
            return QModelIndex();
        return createIndex(row, col, data);
    }
}

QModelIndex MediaModel::parent(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return QModelIndex();
    if (idx.internalPointer() == 0) // top level
        return QModelIndex();
    Data *data = static_cast<Data *>(idx.internalPointer());
    int loc = m_data.indexOf(data);
    if (loc == -1)
        return QModelIndex();
    return createIndex(loc, 0, 0);
}

int MediaModel::columnCount(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return 1;
}

int MediaModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_data.count();
    if (parent.internalPointer() == 0) { // top level
        Data *data = m_data.value(parent.row());
        return data ? data->mediaInfos.count() : 0;
    } else {
        return 0;
    }
}

QVariant MediaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.internalPointer() == 0) { // top level
        Data *data = m_data.value(index.row());
        if (!data)
            return QVariant();
        if (role == Qt::DisplayRole) {
            return data->name;
        } else if (role == Qt::DecorationRole) {
        } else if (role == DecorationUrlRole) {
            return QUrl("image://" + imageBaseUrl() + data->searchPath);
        } else if (role == FilePathRole) {
            return data->searchPath;
        }

        return QVariant();
    }

    Data *d = static_cast<Data *>(index.internalPointer());
    MediaInfo *info = d->mediaInfos[index.row()];
    if (role == DecorationUrlRole) {
        return QUrl("image://" + imageBaseUrl() + info->filePath);
    } else if (role == FilePathRole) {
        return info->filePath;
    } else if (role == FileNameRole) {
        return info->fileName;
    } else {
        if (info->fileName == tr("..")) // plain awful (aka coding against a deadline)
            return role == Qt::DisplayRole ? info->fileName : QVariant();
        else
            return data(info, role);
    }
}

void MediaModel::addMedia(int row, MediaInfo *mi)
{
    Data *data = m_data[row];
    beginInsertRows(createIndex(row, 0, 0), data->mediaInfos.count(), data->mediaInfos.count());
    QImage frontCover = decoration(mi);
    m_frontCovers.insert(mi->filePath, frontCover);
    data->mediaInfos.append(mi);
    endInsertRows();
}

QPixmap MediaModel::decorationPixmap(const QString &_path, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);
    QString path = '/' + _path;
    if (!m_frontCovers.contains(path)) {
        return QPixmap();
    }
    QPixmap pixmap;
    if (!QPixmapCache::find("coverart_" + path, &pixmap)) {
        pixmap = QPixmap::fromImage(m_frontCovers[path]);
        QPixmapCache::insert("coverart_" + path, pixmap);
    }
    *size = pixmap.size();
    return pixmap;
}

QImage MediaModel::decorationImage(const QString &path, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);
    if (!m_frontCovers.contains(path))
        return QImage();
    QImage img = m_frontCovers.value(path);
    *size = img.size();
    return img;
}

void MediaModel::setThemeResourcePath(const QString &themePath)
{
    m_themePath = themePath;
    m_frontCovers.insert("/AddNewSource", QImage(m_themePath + "/media/DefaultAddSource.png"));
    m_frontCovers.insert("/DotDot", QImage(m_themePath + "/media/DefaultFolderBack.png"));
    reset();
}

MediaModelThread::MediaModelThread(MediaModel *model, int row, const QString &searchPath)
    : m_model(model), m_stop(false), m_row(row), m_searchPath(searchPath)
{
}

MediaModelThread::~MediaModelThread()
{
}

void MediaModelThread::stop()
{
    m_stop = true;
}

void MediaModelThread::run()
{
    emit started();

    Q_ASSERT(!m_searchPath.isEmpty() && m_searchPath != tr("/AddNewSource"));

    search();

    emit finished();
}

void MediaModelThread::search()
{
    QDirIterator it(m_searchPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (!m_stop && it.hasNext()) {
        MediaInfo *info = m_model->readMediaInfo(it.next());
        if (!info)
            continue;

        info->filePath = it.filePath();
        info->fileName = it.fileName();

        emit mediaFound(m_row, info);
   }
}

void MediaModel::registerImageProvider(QDeclarativeContext *context)
{
    context->engine()->addImageProvider(imageBaseUrl(), new MediaImageProvider(this));
}

QString MediaModel::typeString() const
{
    int idx = MediaModel::staticMetaObject.indexOfEnumerator("MediaType");
    QMetaEnum e = MediaModel::staticMetaObject.enumerator(idx);
    return QString::fromLatin1(e.valueToKey(m_type));
}

void MediaModel::dump()
{
    qDebug() << m_data.count() << "elements";
    for (int i = 0; i < m_data.count(); i++) {
        qDebug() << m_data[i]->searchPath;
        for(int j = 0; j < m_data[i]->mediaInfos.count(); j++)
            qDebug() << "\t\t" << m_data[i]->mediaInfos[j]->filePath;
    }
}

