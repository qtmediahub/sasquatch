#ifndef QMHUTIL_H
#define QMHUTIL_H

#include <QWidget>
#include "qmh-config.h"

namespace Utils
{
    static void optimizeWidgetAttributes(QWidget *widget, bool transparent = false) {
        widget->setAttribute(Qt::WA_OpaquePaintEvent);
        widget->setAutoFillBackground(false);
        if (transparent && Config::isEnabled("shine-through", false))
            widget->setAttribute(Qt::WA_TranslucentBackground);
        else
            widget->setAttribute(Qt::WA_NoSystemBackground);
    }
};

#endif // QMHUTIL_H
