#ifndef MEDIAPLAYERHELPER_H
#define MEDIAPLAYERHELPER_H

#include <QObject>

#include "../plugins/mediainfo.h"

class MediaPlayerHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MediaInfo* mediaInfo READ mediaInfo NOTIFY mediaInfoChanged) // ToBeRemoved
    Q_PROPERTY(int position READ position NOTIFY mediaInfoChanged) // ToBeRemoved

public:
    explicit MediaPlayerHelper(QObject *parent = 0);

    MediaInfo* mediaInfo() { return m_mediaInfo; } // ToBeRemoved
    double position() { return m_position; } // ToBeRemoved

signals:
    void mediaInfoChanged(); // ToBeRemoved

    void stopRequested();
    void pauseRequested();
    void resumeRequested();
    void togglePlayPauseRequested();
    void nextRequested();
    void previousRequested();

    void playRemoteSourceRequested(MediaInfo *mediaInfo, int position);

public slots:
    void playRemoteSource(QString uri, int position);
    void stop() { emit stopRequested(); }
    void pause() { emit pauseRequested(); }
    void resume() { emit resumeRequested(); }
    void togglePlayPause() { emit togglePlayPauseRequested(); }
    void next() { emit nextRequested(); }
    void previous() { emit previousRequested(); }

private:
    MediaInfo *m_mediaInfo; // ToBeRemoved
    int m_position; // ToBeRemoved
};

#endif // MEDIAPLAYERHELPER_H
