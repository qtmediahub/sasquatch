#ifndef QMLUTILS_H
#define QMLUTILS_H

#include <QObject>
#include <QDeclarativeItem>

class QMLUtils : public QObject
{
    Q_OBJECT
public:
    QMLUtils(QObject *pQmlContainer) : QObject(pQmlContainer), qmlContainer(pQmlContainer) { /**/ }
    Q_INVOKABLE void applyWebViewFocusFix(QDeclarativeItem *item); // See https://bugs.webkit.org/show_bug.cgi?id=51094
private:
    QObject *qmlContainer;
};

#endif // QMLUTILS_H
