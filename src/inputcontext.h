#ifndef INPUTCONTEXTRPC_H
#define INPUTCONTEXTRPC_H

#include <QInputContext>

class InputContext : public QInputContext
{
    Q_OBJECT
public:
    explicit InputContext(QObject *parent = 0);

    virtual bool filterEvent(const QEvent *event);
    virtual QString identifierName() { return QString::fromLatin1("QtMediaHubInputContext"); }
    virtual QString language() { return QString::fromLatin1("en"); }
    virtual void reset();
    virtual bool isComposing() const { return m_composing; }

signals:
    Q_SCRIPTABLE void inputMethodStartRequested();
    Q_SCRIPTABLE void inputMethodStopRequested();

private:
    bool m_composing;
};

#endif // INPUTCONTEXTRPC_H
