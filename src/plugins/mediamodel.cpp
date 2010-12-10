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
#include <QQueue>

MediaModel::MediaModel(MediaModel::MediaType type, QObject *parent)
    : QAbstractItemModel(parent),
      m_type(type),
      m_thread(0),
      m_nowSearching(-1),
      m_root(0)
{
    qRegisterMetaType<MediaInfo *>("MediaInfo *");

    QHash<int, QByteArray> roleNames = QAbstractItemModel::roleNames();
    roleNames[Qt::DisplayRole] = "display";
    roleNames[DecorationUrlRole] = "decorationUrl";
    roleNames[FilePathRole] = "filePath";
    roleNames[FileNameRole] = "fileName";
    roleNames[MediaInfoTypeRole] = "type";
    roleNames[SizeRole] = "size";
    setRoleNames(roleNames);

    m_root = new MediaInfo(MediaModel::Root);

    MediaInfo *addNewSource = new MediaInfo(MediaModel::AddNewSource);
    addNewSource->filePath = "/AddNewSource";
    addNewSource->name = tr("Add new source");
    m_root->children.append(addNewSource);
}

MediaModel::~MediaModel()
{
    qDeleteAll(m_root->children);

    // FIXME: Wait until thread is dead
    delete m_thread;
}

void MediaModel::startSearchThread()
{
    if (m_nowSearching != -1)
        return; // already searching some directory

    QList<MediaInfo *> mediaInfos = m_root->children;
    int i;
    for (i = 0; i < mediaInfos.count(); i++) { // leave out the last item
        if (mediaInfos[i]->type == MediaModel::SearchPath && mediaInfos[i]->status == MediaInfo::NotSearched)
            break;
    }
    if (i == mediaInfos.count()) {
        m_nowSearching = -1;
        return; // all searched
    }

    Q_ASSERT(!m_thread);
    m_nowSearching = i;
    m_thread = new MediaModelThread(this, mediaInfos[i]);
    m_thread->setAutoDelete(false);
    mediaInfos[i]->status = MediaInfo::Searching;
    connect(m_thread, SIGNAL(mediaFound(MediaInfo *)), this, SLOT(addMedia(MediaInfo *)));
    connect(m_thread, SIGNAL(finished()), this, SLOT(searchThreadFinished()));
    QThreadPool::globalInstance()->start(m_thread);
}

void MediaModel::searchThreadFinished()
{
    Q_ASSERT(m_nowSearching != -1);
    Q_ASSERT(m_thread);
    m_root->children[m_nowSearching]->status = MediaInfo::Searched;
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
    beginInsertRows(QModelIndex(), m_root->children.count()-1, m_root->children.count()-1);
    MediaInfo *newSearchPath = new MediaInfo(MediaModel::SearchPath);
    newSearchPath->filePath = path;
    newSearchPath->name = name;
    m_root->children.insert(m_root->children.count()-1, newSearchPath); // add before AddNewSource
    endInsertRows();
    m_frontCovers.insert(path, QImage(m_themePath + "/media/DefaultHardDisk.png"));

    startSearchThread();
}

QModelIndex MediaModel::index(int row, int col, const QModelIndex &parent) const
{
    if (col != 0 || row < 0)
        return QModelIndex();
    MediaInfo *parentInfo = parent.isValid() ? static_cast<MediaInfo *>(parent.internalPointer()) : m_root;
    if (!parentInfo || row >= parentInfo->children.count())
        return QModelIndex();
    return createIndex(row, col, parentInfo->children[row]);
}

QModelIndex MediaModel::parent(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return QModelIndex();
    MediaInfo *info = static_cast<MediaInfo *>(idx.internalPointer());
    MediaInfo *parent = info->parent;
    MediaInfo *grandParent = parent && parent->parent ? parent->parent : m_root;
    return createIndex(grandParent->children.indexOf(parent), idx.column(), parent);
}

int MediaModel::columnCount(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return 1;
}

int MediaModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_root->children.count();
    MediaInfo *info = static_cast<MediaInfo *>(parent.internalPointer());
    return info->children.count();
}

QVariant MediaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    MediaInfo *info = static_cast<MediaInfo *>(index.internalPointer());

    if (role == DecorationUrlRole) {
        return QUrl("image://" + imageBaseUrl() + info->filePath);
    } else if (role == FilePathRole) {
        return info->filePath;
    } else if (role == FileNameRole) {
        return info->name;
    } else if (role == MediaInfoTypeRole) {
        int idx = MediaModel::staticMetaObject.indexOfEnumerator("MediaInfoType");
        QMetaEnum e = MediaModel::staticMetaObject.enumerator(idx);
        return QString::fromLatin1(e.valueToKey(info->type));
    } else if (role == SizeRole) {
        return info->size;
    }

    if (info->parent == m_root) {
        if (role == Qt::DisplayRole)
            return info->name;

        return QVariant();
    } else {
        if (info->type != MediaModel::File)
            return role == Qt::DisplayRole ? info->name : QVariant();
        else
            return data(info, role);
    }
}

void MediaModel::addMedia(MediaInfo *mi)
{
    MediaInfo *parent = mi->parent;
    Q_ASSERT(parent);
    MediaInfo *grandParent = parent->parent ? parent->parent : m_root;
    QModelIndex parentIndex = createIndex(grandParent->children.indexOf(parent), 0, parent);
    beginInsertRows(parentIndex, parent->children.count(), parent->children.count());
    QImage frontCover;
    if (mi->type == MediaModel::File)
        frontCover = decoration(mi);
    else
        frontCover = QImage(m_themePath + "/media/DefaultFolder.png");
    m_frontCovers.insert(mi->filePath, frontCover);
    parent->children.append(mi);
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

MediaModelThread::MediaModelThread(MediaModel *model, MediaInfo *mediaInfo)
    : m_model(model), m_stop(false), m_mediaInfo(mediaInfo), m_searchPath(mediaInfo->filePath)
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
    QQueue<MediaInfo *> dirQ;
    QString currentSearchPath = m_searchPath;
    MediaInfo *currentParent = m_mediaInfo;

    while (true) {
        QDirIterator it(currentSearchPath, QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
        while (!m_stop && it.hasNext()) {
            it.next();
            MediaInfo *info = 0;
            if (it.fileInfo().isDir()) {
                info = new MediaInfo(MediaModel::Directory);
                dirQ.enqueue(info);
            } else if (it.fileInfo().isFile()) {
                info = m_model->readMediaInfo(it.filePath());
                if (!info)
                    continue;
            }
            info->filePath = it.filePath();
            info->name = it.fileName();
            info->parent = currentParent;

            emit mediaFound(info);
        }

        if (dirQ.isEmpty())
            break;

        currentParent = dirQ.dequeue();
        currentSearchPath = currentParent->filePath;
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

void MediaModel::dump(MediaInfo *info, int indent)
{
    qDebug() << info->children.count() << "elements";
    QString space;
    space = space.fill(' ', indent);
    for (int i = 0; i < info->children.count(); i++) {
        qDebug() << qPrintable(space) << info->children[i]->filePath;
        dump(info->children[i], indent+4);
    }
}

