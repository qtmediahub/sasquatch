#ifndef QMHPLUGININTERFACE_H
#define QMHPLUGININTERFACE_H

#include <QObject>
#include <QStringList>

class QDeclarativeContext;

class QMHPluginInterface
{
public:
    QMHPluginInterface()
        : mVisualElement(0),
          mName("Skin Plugin"),
          mRole("undefined") { /**/ }

    virtual ~QMHPluginInterface() { /* */ }

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    QString role() const { return mRole; }
    void setRole(const QString &role) { mRole = role; }

    QObject* visualElement() const { return mVisualElement; }
    void setVisualElement(QObject *element) { mVisualElement = element; }

    QStringList visualElementProperties() const { return mVisualElementProperties; }
    void setVisualElementProperties(const QStringList& properties) {
        mVisualElementProperties = properties;
        //Must be property/value pairs
        if(mVisualElementProperties.size() % 2)
            mVisualElementProperties.removeLast();
    }

    QStringList actionList() const { return mActionList; }
    void setActionList(const QStringList& actions) { mActionList = actions; }

    virtual bool dependenciesSatisfied() const { return true; }

    virtual void registerPlugin(QDeclarativeContext *context) { Q_UNUSED(context); }
    virtual void unregisterPlugin() { /**/ }

    virtual QObject *pluginProperties() const { return 0; }

protected:
    QObject *mVisualElement;
    QStringList mVisualElementProperties;
    QStringList mActionList;
    QString mName;
    QString mRole;
};

Q_DECLARE_INTERFACE(QMHPluginInterface, "com.nokia.QMH.PluginInterface/1.0")

#endif // QMHPLUGININTERFACE_H
