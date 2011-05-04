#include "device.h"

#ifndef QT_NO_DBUS
#include "systemhelperdbus.h"
#endif

Device::Device(const QString &p, DeviceType t, const QString &l, const QString &u, QObject *parent) :
    QObject(parent)
  , m_path(p)
  , m_type(t)
  , m_label(l)
  , m_uuid(u)
{
#ifndef QT_NO_DBUS
    m_helper = qobject_cast<SystemHelperDBus*>(parent);
#endif
    emit changed();
}

void Device::mount()
{
#ifndef QT_NO_DBUS
    m_helper->mount(m_path);
#endif
}

void Device::unmount()
{
#ifndef QT_NO_DBUS
    m_helper->unmount(m_path);
#endif
}

void Device::eject()
{
#ifndef QT_NO_DBUS
    m_helper->eject(m_path);
#endif
}
