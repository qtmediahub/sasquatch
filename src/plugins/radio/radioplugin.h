#ifndef VIDEOPLUGIN_H
#define VIDEOPLUGIN_H

#include <QObject>
#include <QDeclarativeContext>
#include "qmhplugininterface.h"
#include "radiomodel.h"

class RadioPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)
    Q_PROPERTY(QObject *model READ model NOTIFY modelChanged)

public:
    RadioPlugin();
    ~RadioPlugin() {}

    QObject *pluginProperties() const;

    // accessed from QML
    QObject *model() const { return m_model; }

    void registerPlugin(QDeclarativeContext *context);

signals:
    void modelChanged();

private:
    RadioModel *m_model;
};

#endif // VIDEOPLUGIN_H
