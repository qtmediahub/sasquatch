#ifndef VIDEOPLUGIN_H
#define VIDEOPLUGIN_H

#include <QObject>
#include <QDeclarativeContext>
#include "qmhplugin.h"
#include "radiomodel.h"

class RadioPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)
    Q_PROPERTY(QObject *model READ model NOTIFY modelChanged)

public:
    RadioPlugin();
    ~RadioPlugin() {}

    // accessed from QML
    QObject *model() const { return m_model; }

    void registerPlugin(QDeclarativeContext *context);

signals:
    void modelChanged();

private:
    RadioModel *m_model;
};

#endif // VIDEOPLUGIN_H
