#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>

class QProcess;

class ProcessManager : public QObject
{
    Q_OBJECT
public:
    explicit ProcessManager(QObject *parent = 0);
    Q_INVOKABLE void launchProcess(const QString &appName, const QStringList &args);
    Q_INVOKABLE void releaseProcess();
private:
    QProcess *processInstance;
};

#endif // PROCESSMANAGER_H
