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

#ifndef MUSICMODEL_H
#define MUSICMODEL_H

#include <QAbstractListModel>
#include <QImage>
#include <QPixmap>
#include <QPixmapCache>
#include <QRunnable>
#include <QList>
#include <QUrl>

struct MusicInfo 
{
    MusicInfo() : year(0), track(0), length(0), bitrate(0), sampleRate(0), channels(0) { }
    QString filePath;
    QString fileName;
    // tag info
    QString title;
    QString artist;
    QString album;
    QString comment;
    QString genre;
    quint32 year;
    quint32 track;
    // audio properties
    int  length;
    int  bitrate;
    int  sampleRate;
    int  channels;
    // cover art
    QImage frontCover;
    mutable QPixmapCache::Key frontCoverPixmapKey;
};

class MusicModel;

class MusicModelThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    MusicModelThread(MusicModel *model);
    ~MusicModelThread();

    void run();

    void stop();
signals:
    void started();
    void musicFound(const MusicInfo &info);
    void finished();

private:
    MusicModel *m_model;
    bool m_stop;
};

class MusicModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QString musicPath READ musicPath)

public:
    MusicModel(const QString &musicPath, QObject *parent = 0);
    ~MusicModel();

    // reimp
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int col, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &idx) const;
    int columnCount(const QModelIndex &idx) const;

    QString musicPath() const;
    QPixmap decorationPixmap(const QString &path, QSize *size, const QSize &requestedSize);
    QImage decorationImage(const QString &path, QSize *size, const QSize &requestedSize);

    enum CustomRoles {
        TitleRole = Qt::UserRole + 1,
        ArtistRole,
        AlbumRole,
        CommentRole,
        GenreRole,
        FilePathRole,
        FileNameRole,
        DecorationUrlRole
    };

    Q_INVOKABLE void setThemeResourcePath(const QString &themePath);
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

public slots:
    void addMusic(const MusicInfo &music);

signals:
    void musicPathChanged();

private:
    void init();
    QPixmap decorationPixmap(MusicInfo *info) const;
    QString m_musicPath;
    QList<MusicInfo> m_musicInfos;
    QHash<QString, int> m_pathToIndex;
    MusicModelThread *m_thread;
    QString m_themePath;
    QImage m_fanartFallbackImage;
    mutable QPixmapCache::Key m_fanartFallbackKey;
};

#endif // MUSICMODEL_H

