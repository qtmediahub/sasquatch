#include <QWidget>
#include "qmh-config.h"

namespace Utils
{
    void optimizeWidgetAttributes(QWidget *widget, bool transparent = false) {
        widget->setAttribute(Qt::WA_OpaquePaintEvent);
        widget->setAutoFillBackground(false);
        if (transparent && Config::isEnabled("shine-through", false))
            widget->setAttribute(Qt::WA_TranslucentBackground);
        else
            widget->setAttribute(Qt::WA_NoSystemBackground);
    }

    void optimizeGraphicsViewAttributes(QGraphicsView *view)
    {
        if (Config::isEnabled("smooth-scaling", false))
            view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setFrameStyle(0);
        view->setOptimizationFlags(QGraphicsView::DontSavePainterState);
        view->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    }
};
