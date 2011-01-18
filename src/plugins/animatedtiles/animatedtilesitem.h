#ifndef ANIMATEDTILESITEM_H
#define ANIMATEDTILESITEM_H

#include <QDeclarativeItem>

class AnimatedTilesItem : public QDeclarativeItem
{
    Q_OBJECT
public:
    explicit AnimatedTilesItem(QDeclarativeItem *parent = 0);

    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

private:
    void setupAT();
    QGraphicsRectItem* parentItem;
};

#endif // ANIMATEDTILESITEM_H