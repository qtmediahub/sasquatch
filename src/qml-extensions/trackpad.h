#ifndef TRACKPAD_H 
#define TRACKPAD_H

#include <QObject>

class Frontend;

class Trackpad : public QObject
{
    Q_OBJECT

public:
    explicit Trackpad(Frontend *frontend = 0);
    ~Trackpad();

public slots:
    void setEnabled(bool enable);
    void moveBy(int x, int y);
    void click();

private:
    Frontend *m_frontend;
};

#endif // TRACKPAD_H

