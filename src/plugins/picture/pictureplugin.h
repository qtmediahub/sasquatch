#ifndef PICTUREPLUGIN_H
#define PICTUREPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"
#include "picturemodel.h"

class PicturePlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)
    Q_PROPERTY(QObject *model READ model NOTIFY modelChanged)

public:
    PicturePlugin();
    ~PicturePlugin() {}

    QObject *pluginProperties() const;
        
    // accessed from QML
    QObject *model() const { return m_model; }
    
    void registerPlugin(QDeclarativeContext *context);
signals:
    void modelChanged();
    
private:
    PictureModel *m_model;
};

#endif // PICTUREPLUGIN_H
