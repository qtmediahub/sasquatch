#ifndef PICTUREPLUGIN_H
#define PICTUREPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"
#include "picturemodel.h"

class PicturePlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)
    Q_PROPERTY(QObject *pictureModel READ pictureModel NOTIFY pictureModelChanged)

public:
    PicturePlugin();
    ~PicturePlugin() {}
    QString name() const { return tr("Pictures"); }
    QString role() const { return "picture"; }

    QObject *pluginProperties() const;
        
    // accessed from QML
    QObject *pictureModel() const { return m_model; }
    
    void registerPlugin(QDeclarativeContext *context);
signals:
    void pictureModelChanged();
    
private:
    PictureModel *m_model;
};

#endif // PICTUREPLUGIN_H
