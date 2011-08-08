#include "testingbackend.h"
#ifdef XINE_BACKEND
#include "xinebackend.h"
#endif

#include "qtsingleapplication.h"
#include <QDebug>
#include <QtDBus>

int main(int argc, char** argv)
{
    QtSingleApplication app(argc, argv);
    app.setApplicationName("mediahelper");
    app.setOrganizationName("Nokia");
    app.setOrganizationDomain("nokia.com");

    if (app.isRunning()) {
        qWarning() << app.applicationName() << "is already running, aborting";
        return false;
    }

    bool dbusRegistration = QDBusConnection::sessionBus().registerService(QMH_HELPER_DBUS_SERVICENAME);
    if (!dbusRegistration) {
        qDebug()
            << "Can't seem to register dbus service:"
            << QDBusConnection::sessionBus().lastError().message();
        app.exit(-1);
    }
    MediaBackendInterface *player = 0;
#ifdef XINE_BACKEND
    player = new XineBackend(&app);
#else
    player = new TestingBackend(&app);
#endif

    dbusRegistration = QDBusConnection::sessionBus().registerObject("/", player,
            QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals);

    if (!dbusRegistration) {
        qDebug()
            << "Can't seem to register dbus object:"
            << QDBusConnection::sessionBus().lastError().message();
        app.exit(-1);
    }
    return app.exec();
}
