#include "qml-utils.h"

#include <QGraphicsWidget>
#include <QDeclarativeView>

void QMLUtils::applyWebViewFocusFix(QDeclarativeItem *item) // See https://bugs.webkit.org/show_bug.cgi?id=51094
{
#if QT_VERSION < 0x040800
    item->setFlag(QGraphicsItem::ItemIsFocusScope, true);
    QList<QGraphicsItem *> children = item->childItems();
    for (int i = 0; i < children.count(); i++) {
        if (QGraphicsWidget *maybeWebView = qgraphicsitem_cast<QGraphicsWidget *>(children[i])) {
            if (maybeWebView->inherits("QGraphicsWebView"))
                maybeWebView->setFocus();
        }
    }
#else
    Q_UNUSED(item)
#endif
}


