#ifndef DECLARATIVEVIEW_H
#define DECLARATIVEVIEW_H

#include <QDeclarativeView>
#include <QElapsedTimer>

class DeclarativeView : public QDeclarativeView
{
    Q_OBJECT
public:
    DeclarativeView(QWidget *parent = 0);
    void setSource(const QUrl &url);

protected:
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent *event);

public slots:
    void handleSourceChanged();
    void handleStatusChanged(QDeclarativeView::Status status);

signals:
    void fpsCap(int);

private:
    int m_frameCount;
    int m_timeSigma;
    QElapsedTimer m_frameTimer;
    QUrl m_url;
};

#endif // DECLARATIVEVIEW_H

