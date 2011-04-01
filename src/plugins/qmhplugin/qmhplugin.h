#ifndef QMHPLUGIN_H
#define QMHPLUGIN_H

#include <QObject>
#include <QStringList>

class QDeclarativeContext;

class QMHPlugin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY pluginChanged)
    Q_PROPERTY(QString role READ role WRITE setRole NOTIFY pluginChanged)
    Q_PROPERTY(QObject* visualElement READ visualElement WRITE setVisualElement NOTIFY pluginChanged)
    Q_PROPERTY(QStringList visualElementProperties READ visualElementProperties WRITE setVisualElementProperties NOTIFY pluginChanged)
    Q_PROPERTY(QObject* actionMap READ actionMap WRITE setActionMap NOTIFY pluginChanged)
    Q_PROPERTY(QStringList actionList READ actionList WRITE setActionList NOTIFY pluginChanged)
    Q_PROPERTY(QObject* pluginProperties READ pluginProperties NOTIFY pluginChanged)

public:
    QMHPlugin(QObject *parent = 0)
        : QObject(parent),
          mAdvertized(true),
          mVisualElement(0),
          mActionMap(0)
    {}

    virtual ~QMHPlugin() { /*no impl*/ }

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; emit pluginChanged(); }

    QString role() const { return mRole; }
    void setRole(const QString &role) { mRole = role; emit pluginChanged(); }

    bool advertized() const { return mAdvertized; }
    void setAdvertized(bool advertized) { mAdvertized = advertized; emit pluginChanged(); }

    QObject* visualElement() const { return mVisualElement; }
    void setVisualElement(QObject *element) { mVisualElement = element; emit pluginChanged(); }

    //Not constraining to pairs, this should go in any case
    QStringList visualElementProperties() const { return mVisualElementProperties; }
    void setVisualElementProperties(const QStringList& properties) { mVisualElementProperties = properties; emit pluginChanged(); }

    QObject* actionMap() const { return mActionMap; }
    void setActionMap(QObject *map) { mActionMap = map; emit pluginChanged(); }

    QStringList actionList() const { return mActionList; }
    void setActionList(const QStringList& actions) { mActionList = actions; emit pluginChanged(); }

    //These plugins should be equally usable from html
    virtual void registerPlugin(QDeclarativeContext *context = 0) { Q_UNUSED(context); }
    virtual void unregisterPlugin() { /*no impl*/ }

    virtual bool dependenciesSatisfied() const { return true; }

    virtual QObject *pluginProperties() const { return 0; }

signals:
    void pluginChanged();

protected:
    QString mName;
    QString mRole;
    bool mAdvertized;
    QObject *mVisualElement;
    QStringList mVisualElementProperties;
    QObject *mActionMap;
    QStringList mActionList;
};

Q_DECLARE_INTERFACE(QMHPlugin, "com.nokia.QMH.Plugin/1.0")

#endif // QMHPLUGIN_H
