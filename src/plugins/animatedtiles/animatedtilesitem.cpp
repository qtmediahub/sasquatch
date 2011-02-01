#include <QParallelAnimationGroup>
#include <QStateMachine>

#include <QtGui>
#include <QtCore/qstate.h>

#include "animatedtilesitem.h"
#include "animatedtileshelper.h"


AnimatedTilesItem::AnimatedTilesItem(QDeclarativeItem *parent) :
    QDeclarativeItem(parent)
{
   setupAT();
}

void AnimatedTilesItem::setupAT()
{
    parentItem = new QGraphicsRectItem(this);

    QPixmap kineticPix(":/images/kinetic.png");
    QList<Pixmap *> items;
    for (int i = 0; i < 64; ++i) {
        Pixmap *item = new Pixmap(kineticPix, parentItem);
        item->setOffset(-kineticPix.width()/2, -kineticPix.height()/2);
        item->setZValue(i);
        item->setPos(i*10,i*10);
        items << item;
    }

    // Buttons
    QGraphicsItem *buttonParent = new QGraphicsRectItem(parentItem);
    Button *ellipseButton = new Button(QPixmap(":/images/ellipse.png"), buttonParent);
    Button *figure8Button = new Button(QPixmap(":/images/figure8.png"), buttonParent);
    Button *randomButton = new Button(QPixmap(":/images/random.png"), buttonParent);
    Button *tiledButton = new Button(QPixmap(":/images/tile.png"), buttonParent);
    Button *centeredButton = new Button(QPixmap(":/images/centered.png"), buttonParent);

    ellipseButton->setPos(-100, -100);
    figure8Button->setPos(100, -100);
    randomButton->setPos(0, 0);
    tiledButton->setPos(-100, 100);
    centeredButton->setPos(100, 100);

//    scene.addItem(buttonParent); give 'this' as parent instead
    buttonParent->scale(0.75, 0.75);
    buttonParent->setPos(220, 220);
    buttonParent->setZValue(65);

    // States
    QState *rootState = new QState;
    QState *ellipseState = new QState(rootState);
    QState *figure8State = new QState(rootState);
    QState *randomState = new QState(rootState);
    QState *tiledState = new QState(rootState);
    QState *centeredState = new QState(rootState);

    // Values
    for (int i = 0; i < items.count(); ++i) {
        Pixmap *item = items.at(i);
        // Ellipse
        ellipseState->assignProperty(item, "pos",
                                         QPointF(cos((i / 63.0) * 6.28) * 250,
                                                 sin((i / 63.0) * 6.28) * 250));

        // Figure 8
        figure8State->assignProperty(item, "pos",
                                         QPointF(sin((i / 63.0) * 6.28) * 250,
                                                 sin(((i * 2)/63.0) * 6.28) * 250));

        // Random
        randomState->assignProperty(item, "pos",
                                        QPointF(-250 + qrand() % 500,
                                                -250 + qrand() % 500));

        // Tiled
        tiledState->assignProperty(item, "pos",
                                       QPointF(((i % 8) - 4) * kineticPix.width() + kineticPix.width() / 2,
                                               ((i / 8) - 4) * kineticPix.height() + kineticPix.height() / 2));

        // Centered
        centeredState->assignProperty(item, "pos", QPointF());
    }


    QStateMachine* states = new QStateMachine(this);
    states->addState(rootState);
    states->setInitialState(rootState);
    rootState->setInitialState(figure8State);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    for (int i = 0; i < items.count(); ++i) {
        QPropertyAnimation *anim = new QPropertyAnimation(items[i], "pos");
        anim->setDuration(750 + i * 25);
        anim->setEasingCurve(QEasingCurve::InOutBack);
        group->addAnimation(anim);
    }
    QAbstractTransition *trans = rootState->addTransition(this, SIGNAL(ellipseWanted()), ellipseState);
    trans->addAnimation(group);

    trans = rootState->addTransition(this, SIGNAL(figure8Wanted()), figure8State);
    trans->addAnimation(group);

    trans = rootState->addTransition(this, SIGNAL(randomWanted()), randomState);
    trans->addAnimation(group);

    trans = rootState->addTransition(this, SIGNAL(tiledWanted()), tiledState);
    trans->addAnimation(group);

    trans = rootState->addTransition(this, SIGNAL(centeredWanted()), centeredState);
    trans->addAnimation(group);

    connect(ellipseButton, SIGNAL(pressed()), this, SIGNAL(ellipseWanted()));
    connect(figure8Button, SIGNAL(pressed()), this, SIGNAL(figure8Wanted()));
    connect(randomButton, SIGNAL(pressed()), this, SIGNAL(randomWanted()));
    connect(tiledButton, SIGNAL(pressed()), this, SIGNAL(tiledWanted()));
    connect(centeredButton, SIGNAL(pressed()), this, SIGNAL(centeredWanted()));


    states->start();
}

void AnimatedTilesItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    parentItem->setPos(newGeometry.width()/2.0, newGeometry.height()/2.0);
    double scale = qMin<double>(newGeometry.width()/700.0, newGeometry.height()/700);
    parentItem->setScale(scale);
}
