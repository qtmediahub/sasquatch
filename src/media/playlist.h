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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QtGui>
#include <QtSql>

class MediaModel;

class Playlist : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(PlayMode playMode READ playMode WRITE setPlayMode NOTIFY playModeChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

    Q_ENUMS(PlayMode)

public:
    enum PlayMode {
        Normal,
        Shuffle
    };

    Playlist(QObject *parent = 0);

    Q_INVOKABLE void add(MediaModel *mediaModel, int row);
    Q_INVOKABLE void addCurrentLevel(MediaModel *mediaModel);
    Q_INVOKABLE void clear();

    Q_INVOKABLE int next();
    Q_INVOKABLE int previous();

    QString name() const;
    void setName(const QString &name);

    PlayMode playMode() const;
    void setPlayMode(PlayMode mode);

    void setCurrentIndex(int idx);
    int currentIndex() const;

    // reimp
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    Q_INVOKABLE QVariant data(int idx, const QString &role) const;

private slots:
    void saveToDatabase();
    void loadFromDatabase();

signals:
    void nameChanged();
    void playModeChanged();
    void currentIndexChanged();

private:
    void saveLater();

    QList<QMap<int, QVariant> > m_data;
    QString m_name;
    PlayMode m_playMode;
    int m_currentIndex;
    QTimer m_saveTimer;
};

#endif // PLAYLIST_H
