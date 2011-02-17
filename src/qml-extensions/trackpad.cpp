#include "trackpad.h"
#include "../frontend.h"
#include <QtGui>
#include <QtDeclarative>

Trackpad::Trackpad(Frontend *frontend)
    : QObject(frontend), m_frontend(frontend)
{
    setObjectName("trackpad");
}

Trackpad::~Trackpad()
{
}

void Trackpad::setEnabled(bool e)
{
    QDeclarativeView *view = qobject_cast<QDeclarativeView *>(m_frontend->centralWidget());
    QDeclarativeExpression expression(view->rootContext(), 0, QString("cursor.enableCursor(%1)").arg(e));
    expression.evaluate();
    if (expression.hasError())
        qWarning() << "Failed to enable/disable cursor";
}

void Trackpad::moveBy(int x, int y)
{
    QDeclarativeView *view = qobject_cast<QDeclarativeView *>(m_frontend->centralWidget());
    QDeclarativeExpression expression(view->rootContext(), 0, QString("cursor.moveBy(%1,%2)").arg(x).arg(y));
    expression.evaluate();
    if (expression.hasError())
        qWarning() << "Failed to enable/disable cursor";
}
