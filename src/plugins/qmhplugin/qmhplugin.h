#ifndef QMHPLUGIN_H
#define QMHPLUGIN_H

#include <QObject>
#include <QStringList>

class QMHPlugin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY pluginChanged)
    Q_PROPERTY(bool browseable READ browseable WRITE setBrowseable NOTIFY pluginChanged)
    Q_PROPERTY(QString role READ role WRITE setRole NOTIFY pluginChanged)
    Q_PROPERTY(QList<QObject*> childItems READ childItems NOTIFY pluginChanged)
    Q_PROPERTY(QObject* visualElement READ visualElement WRITE setVisualElement NOTIFY pluginChanged)
    Q_PROPERTY(QStringList visualElementProperties READ visualElementProperties WRITE setVisualElementProperties NOTIFY pluginChanged)

public:
    QMHPlugin(QObject *parent = 0):
        QObject(parent),
        mName("Skin Plugin"),
        mBrowseable(false),
        mRole("undefined"),
        mVisualElement(0) { /* */ }

    virtual ~QMHPlugin() { /* */ }

    virtual QString name() const { return mName; }
    virtual void setName(const QString &name) { mName = name; }

    virtual bool browseable() const { return mBrowseable; }
    virtual void setBrowseable(bool browseable) { mBrowseable = browseable; }

    virtual QString role() const { return mRole; }
    virtual void setRole(const QString &role) { mRole = role; }

    virtual QObject* visualElement() const { return mVisualElement; }
    virtual void setVisualElement(QObject *element) { mVisualElement = element; }

    QList<QObject*> childItems() const { return mChildItems; }

    virtual QStringList visualElementProperties() const { return mVisualElementProperties; }
    virtual void setVisualElementProperties(const QStringList& properties) {
        mVisualElementProperties = properties;
        //Must be property/value pairs
        if(mVisualElementProperties.size() % 2)
            mVisualElementProperties.removeLast();
    }

    virtual void registerPlugin() { /**/ }
    virtual void unregisterPlugin() { /**/ }

signals:
    void pluginChanged();

protected:
    QString mName;
    bool mBrowseable;
    QString mRole;
    QObject *mVisualElement;
    QStringList mVisualElementProperties;
    QList<QObject *> mChildItems;
};

Q_DECLARE_INTERFACE(QMHPlugin, "com.nokia.QMH.Plugin/1.0")

#endif // QMHPLUGIN_H
