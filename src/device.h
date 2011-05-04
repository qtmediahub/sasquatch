#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>

#ifndef QT_NO_DBUS
class SystemHelperDBus;
#endif

class Device : public QObject
{
    Q_OBJECT
    Q_ENUMS(DeviceType)
    Q_PROPERTY(QString path READ path NOTIFY changed)
    Q_PROPERTY(DeviceType type READ type NOTIFY changed)
    Q_PROPERTY(QString label READ label NOTIFY changed)
    Q_PROPERTY(QString uuid READ uuid NOTIFY changed)

public:
    enum DeviceType { Undefined, UsbDrive, Dvd, AudioCd, TypeCount };

    Device(const QString &p, DeviceType t, const QString &l, const QString &u, QObject *parent = 0);

    QString path() const { return m_path; }
    DeviceType type() const { return m_type; }
    QString label() const { return m_label; }
    QString uuid() const { return m_uuid; }

signals:
    void changed();

public slots:
    void mount();
    void unmount();
    void eject();

private:
    QString m_path;
    DeviceType m_type;
    QString m_label;
    QString m_uuid;

#ifndef QT_NO_DBUS
    SystemHelperDBus *m_helper;
#endif
};

#endif // DEVICE_H
