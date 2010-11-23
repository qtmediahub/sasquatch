#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QObject>
#include <QDir>
#include <QDebug>

#include <QDeclarativeItem>

class Dashboard : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(QString widgetPath READ widgetPath WRITE setWidgetPath)

public:
    explicit Dashboard(QDeclarativeItem *parent = 0);
    QString widgetPath() const { return mWidgetPath; }
    void setWidgetPath(const QString &path) { mWidgetPath = path; }
    Q_INVOKABLE QStringList discoverWidgets();
private:
    QString mWidgetPath;
};

#endif // DASHBOARD_H
