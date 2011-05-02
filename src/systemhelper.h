#ifndef SYSTEMHELPER_H
#define SYSTEMHELPER_H

#include <QObject>

class SystemHelper : public QObject
{
    Q_OBJECT
public:
    explicit SystemHelper(QObject *parent = 0);

signals:

public slots:
    void shutdown();
    void restart();
    void suspend();
    void hibernate();

private:

};

#endif // SYSTEMHELPER_H
