#ifndef WINSENDINPUTEVENTHANDLER_H
#define WINSENDINPUTEVENTHANDLER_H

#include <QObject>

#include "baseeventhandler.h"

#include <joybuttonslot.h>


class WinSendInputEventHandler : public BaseEventHandler
{
    Q_OBJECT
public:
    explicit WinSendInputEventHandler(QObject *parent = 0);

    virtual bool init();
    virtual bool cleanup();
    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseEvent(int xDis, int yDis);
    virtual QString getName();
    virtual QString getIdentifier();
    virtual void sendMouseSpringEvent(unsigned int xDis, unsigned int yDis,
                                      unsigned int width, unsigned int height);
    virtual void sendTextEntryEvent(QString maintext);

signals:

public slots:

};

#endif // WINSENDINPUTEVENTHANDLER_H
