#include "processmanager.h"

#include <QProcess>

ProcessManager::ProcessManager(QObject *parent)
    : QObject(parent),
      processInstance(new QProcess(this))
{

}

void ProcessManager::launchProcess(const QString &appName, const QStringList &args) {
    processInstance->kill();
    processInstance->start(appName, args);
}

void ProcessManager::releaseProcess() {
    processInstance->kill();
}
