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

struct VideoInfo
{
    VideoInfo() : length(0), subtitles(0), channels(0) { }
    QString filePath;
    QString fileName;
    // video properties
    int length;
    int subtitles;
    int channels;
    QString decorationUrl;
};

class VideoModel;

class VideoModelThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    VideoModelThread(VideoModel *model, int row, const QString &searchPath);
    ~VideoModelThread();

    void run();

    void stop();

signals:
    void started();
    void videoFound(int row, const VideoInfo &info);
    void finished();

private:
    void search();
    VideoModel *m_model;
    bool m_stop;
    int m_row;
    QString m_searchPath;
};

class VideoModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    VideoModel(QObject *parent = 0);
    ~VideoModel();

    // reimp
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int col, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &idx) const;
    int columnCount(const QModelIndex &idx) const;

    enum CustomRoles {
        FilePathRole = Qt::UserRole + 1,
        FileNameRole,
        DecorationUrlRole,
        LengthRole,
        SubtitlesRole,
        ChannelsRole
    };

    // callable from QML
    Q_INVOKABLE void setThemeResourcePath(const QString &themePath);
    Q_INVOKABLE void addSearchPath(const QString &videoPath, const QString &name);

    void dump();

private slots:
    void addVideo(int row, const VideoInfo &video);
    void searchThreadFinished();

signals:
    void videoPathChanged();

private:
    void init();
    void startSearchThread();
    void stopSearchThread();

    struct Data {
        Data(const QString &sp, const QString &name) : searchPath(sp), name(name), status(NotSearched) { }
        QString searchPath;
        QString name;
        QList<VideoInfo *> videoInfos;
        enum Status { NotSearched, Searching, Searched } status;
    };
    QList<Data *> m_data;
    VideoModelThread *m_thread;
    QString m_themePath;
    QString m_fanartFallbackImagePath;
    int m_nowSearching;
    friend class VideoModelThread;
};

#endif // MUSICMODEL_H

