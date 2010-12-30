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

#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include <QAbstractListModel>
#include <QImage>
#include <QPixmap>
#include <QPixmapCache>
#include <QRunnable>
#include <QList>
#include <QUrl>
#include <QDeclarativeImageProvider>
#include <QDeclarativeContext>
#include <QtDebug>
#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>

class MediaModel;
class MediaInfo;

class MediaModelThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    MediaModelThread(MediaModel *model, MediaInfo *info);
    ~MediaModelThread();

    MediaInfo *mediaInfo() const { return m_mediaInfo; }
    MediaModel *mediaModel() const { return m_model; }

    void run();

    void stop();

signals:
    void started();
    void mediaFound(MediaInfo *info);
    void finished();
    void progress(const QString &path);

private:
    void search();
    MediaModel *m_model;
    bool m_stop;
    MediaInfo *m_mediaInfo;
    QString m_searchPath;
};

class MediaModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_ENUMS(MediaType)
    Q_ENUMS(MediaInfoType)
    Q_ENUMS(SortField)
    Q_PROPERTY(QString currentScanPath READ currentScanPath NOTIFY currentScanPathChanged)

public:
    enum MediaType {
        Music,
        Picture,
        Video
    };

    // this is here, so we can expose it to QML
    enum MediaInfoType {
        Deleted,
        Root, 
        AddNewSource, 
        SearchPath, 
        DotDot, 
        Directory, 
        File 
    };

    enum SortField {
        Name,
        Date,
        Size,
    };

    MediaModel(MediaType type, QObject *parent = 0);
    ~MediaModel();

    // reimp
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int col, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &idx) const;
    int columnCount(const QModelIndex &idx) const;

    QPixmap previewPixmap(const QString &path, QSize *size, const QSize &requestedSize);
    QImage previewImage(const QString &path, QSize *size, const QSize &requestedSize);

    enum CustomRoles {
        // Qt::UserRole+1 to 100 are reserved by this model!
        PreviewUrlRole = Qt::UserRole + 1,
        PreviewWidthRole,
        PreviewHeightRole,
        FilePathRole,
        FileNameRole,
        FileUrlRole,
        MediaInfoTypeRole,
        FileSizeRole,
        FileDateTimeRole
    };

    QString themeResourcePath() const { return m_themePath; }

    // callable from QML
    Q_INVOKABLE void setThemeResourcePath(const QString &themePath);
    Q_INVOKABLE void addSearchPath(const QString &mediaPath, const QString &name);
    Q_INVOKABLE void removeSearchPath(int index);
    Q_INVOKABLE void sort(const QModelIndex &root, const QString &field);
    Q_INVOKABLE void rescan(int index);

    QString typeString() const;
    void registerImageProvider(QDeclarativeContext *context);
    QString imageBaseUrl() const { return typeString().toLower() + "model"; } // ## toLower() needed because of QTBUG-15905

    void dump(const MediaInfo *info, int indent = 0) const;

    virtual MediaInfo *readMediaInfo(const QString &filePath) = 0; // called from thread
    virtual QVariant data(MediaInfo *info, int role) const = 0;

    QString currentScanPath() const;

    QFileInfo generateThumbnailFileInfo(const QFileInfo &fileInfo);
    
private slots:
    void addMedia(MediaInfo *media);
    void searchThreadFinished();
    void setCurrentScanPath(const QString &dir);

signals:
    void currentScanPathChanged();

private:
    void restore();
    void startSearchThread();
    void stopSearchThread();

    QPixmap previewPixmap(MediaInfo *info) const;

    MediaType m_type;
    QHash<QString, QImage> m_defaultThumbnails;
    MediaModelThread *m_thread;
    QString m_fanartFallbackImagePath;
    QString m_themePath;
    int m_nowSearching;
    MediaInfo *m_root;
    QList<MediaInfo *> m_deleteLaterInfos;
    bool m_restored;
    QString m_currentScanPath;
    friend class MediaModelThread;
};

// ##: Maybe this model can multiple inherit from ImageProvider. But ownership of the provider 
// is not clear.
class MediaImageProvider : public QDeclarativeImageProvider
{
public:
    MediaImageProvider(MediaModel *model) 
        : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap),
          m_model(model)
    {
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize)
    {
        return m_model->previewImage(id, size, requestedSize);
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
    {
        return m_model->previewPixmap(id, size, requestedSize);
    }

private:
    MediaModel *m_model;
};

struct MediaInfo
{
    enum Status { NotSearched, Searching, Searched };
    MediaInfo(MediaModel::MediaInfoType type) : parent(0), type(type), status(NotSearched), fileSize(-1) {
        // is this wise?
        if (type == MediaModel::Directory || type == MediaModel::SearchPath) {
            MediaInfo *info = new MediaInfo(MediaModel::DotDot);
            info->name = QT_TRANSLATE_NOOP("MediaModel", "..");
            info->parent = this;
            children.append(info);
        }
    }
    ~MediaInfo() { qDeleteAll(children); children.clear(); }

    MediaInfo *parent;
    MediaModel::MediaInfoType type;
    QString filePath;
    QString name;
    Status status;
    QList<MediaInfo *> children;
    qint64 fileSize;
    QDateTime fileDateTime;
    QSize previewSize;
    QString thumbnail;
};

#endif // MEDIAMODEL_H

