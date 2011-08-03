#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTimer>

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    enum ScreenOrientation {
        ScreenOrientationLockPortrait,
        ScreenOrientationLockLandscape,
        ScreenOrientationAuto
    };
    MainWindow(QWidget *prey);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *e);

signals:
    void shrink();
    void grow();
    void toggleFullScreen();

public slots:
    void setOrientation(ScreenOrientation orientation);
    void handleResize();
    void resetUI();

private:
    QTimer resizeSettleTimer;
    QWidget *m_prey;
};

#endif // MAINWINDOW_H
