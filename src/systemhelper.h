#ifndef SYSTEMHELPER_H
#define SYSTEMHELPER_H

#include <QObject>

class Device;

#ifndef QT_NO_DBUS
class SystemHelperDBus;
#endif

class SystemHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
public:
    explicit SystemHelper(QObject *parent = 0);

    QString name() const { return m_name; }
    void setName(const QString &n) { if (m_name != n) m_name = n; emit nameChanged(); }

    Q_INVOKABLE QObject *getDeviceByPath(const QString &path);

signals:
    void deviceAdded(QString device);
    void deviceRemoved(QString device);
    void nameChanged();

public slots:
    void shutdown();
    void restart();
    void suspend();
    void hibernate();
    void mount(const QString &device);

private:
    QString m_name;

#ifndef QT_NO_DBUS
    SystemHelperDBus *m_helper;
#endif
};

#endif // SYSTEMHELPER_H
