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

    Q_INVOKABLE void setEnabled(bool enable);
    Q_INVOKABLE void moveBy(int x, int y);

private:
    Frontend *m_frontend;
};

#endif // TRACKPAD_H

