#include "widgetwrapper.h"
#include "qmh-config.h"
#include "qmh-util.h"
#include "backend.h"

#include <QGraphicsView>
#include <QDeclarativeView>
#include <QShortcut>

WidgetWrapper::WidgetWrapper(QWidget *prey)
    : QWidget(0),
      m_prey(prey),
      viewport(0)
{
    QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(prey);

    if (scrollArea)
        viewport = scrollArea->viewport();

    prey->setParent(this);

    setOrientation(Config::value("orientation", ScreenOrientationAuto));

    Utils::optimizeWidgetAttributes(this, true);
    Utils::optimizeWidgetAttributes(prey, true);

    if (viewport) {
        //Does not appear to work here
        //Not sure why
        Utils::optimizeWidgetAttributes(viewport, true);
    }

    installEventFilter(Backend::instance());

    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::META + Qt::ALT + Qt::Key_Backspace), this, SLOT(resetUI()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::META + Qt::ALT + Qt::Key_Down), this, SIGNAL(shrink()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::META + Qt::ALT + Qt::Key_Up), this, SIGNAL(grow()));
    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Return), this, SIGNAL(toggleFullScreen()));

    resizeSettleTimer.setSingleShot(true);

    connect(&resizeSettleTimer, SIGNAL(timeout()), this, SLOT(handleResize()));
}

WidgetWrapper::~WidgetWrapper()
{
    delete m_prey;
}

void WidgetWrapper::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    static int staggerResizingDelay = Config::value("resizeDelay", 0);
    resizeSettleTimer.start(staggerResizingDelay);
}

void WidgetWrapper::setOrientation(ScreenOrientation orientation)
{
#if defined(Q_OS_SYMBIAN)
    // If the version of Qt on the device is < 4.7.2, that attribute won't work
    if (orientation != ScreenOrientationAuto) {
        const QStringList v = QString::fromAscii(qVersion()).split(QLatin1Char('.'));
        if (v.count() == 3 && (v.at(0).toInt() << 16 | v.at(1).toInt() << 8 | v.at(2).toInt()) < 0x040702) {
            qWarning("Screen orientation locking only supported with Qt 4.7.2 and above");
            return;
        }
    }
#endif // Q_OS_SYMBIAN

    Qt::WidgetAttribute attribute;
    switch (orientation) {
#if QT_VERSION < 0x040702
    // Qt < 4.7.2 does not yet have the Qt::WA_*Orientation attributes
    case ScreenOrientationLockPortrait:
        attribute = static_cast<Qt::WidgetAttribute>(128);
        break;
    case ScreenOrientationLockLandscape:
        attribute = static_cast<Qt::WidgetAttribute>(129);
        break;
    default:
    case ScreenOrientationAuto:
        attribute = static_cast<Qt::WidgetAttribute>(130);
        break;
#else // QT_VERSION < 0x040702
    case ScreenOrientationLockPortrait:
        attribute = Qt::WA_LockPortraitOrientation;
        break;
    case ScreenOrientationLockLandscape:
        attribute = Qt::WA_LockLandscapeOrientation;
        break;
    default:
    case ScreenOrientationAuto:
        attribute = Qt::WA_AutoOrientation;
        break;
#endif // QT_VERSION < 0x040702
    };
    setAttribute(attribute, true);
}

void WidgetWrapper::handleResize()
{
    m_prey->setFixedSize(size());

    //We are deliberately avoiding this functionality for QML
    //since it handles it indepedently
    QGraphicsView *gv = (QString(m_prey->metaObject()->className()).compare("QGraphicsView") == 0) ? qobject_cast<QGraphicsView*>(m_prey) : 0;
    if (gv && Config::isEnabled("scale-ui", false)) {
        gv->resetMatrix();
        gv->scale(qreal(width())/1280, qreal(height())/720);
    }
}

void WidgetWrapper::resetUI()
{
    QDeclarativeView *declarativeWidget = qobject_cast<QDeclarativeView*>(m_prey);
    if (declarativeWidget) {
        QObject* coreObject = declarativeWidget->rootObject();
        QMetaObject::invokeMethod(coreObject, "reset");
    }
}
