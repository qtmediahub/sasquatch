#ifndef SUBMENUENTRY_H
#define SUBMENUENTRY_H

#include <QObject>

class SubMenuEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    explicit SubMenuEntry(QObject *parent = 0);
    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

signals:
    void nameChanged();

private:
    QString mName;
};

#endif // SUBMENUENTRY_H
