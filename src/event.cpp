//#include <QDebug>
#include <QVariant>
#include <QApplication>
#include <QTime>
#include <cmath>
#include <QFileInfo>
#include <QStringList>
#include <QCursor>
#include <QDesktopWidget>
#include <QProcess>

#include "event.h"

#include "eventhandlerfactory.h"


#if defined(Q_OS_UNIX)

    #if defined(WITH_X11)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include "x11extras.h"

        #ifdef WITH_XTEST
#include <X11/extensions/XTest.h>
        #endif
    #endif

    #if defined(WITH_UINPUT)
#include "uinputhelper.h"
    #endif

#elif defined (Q_OS_WIN)
#include <qt_windows.h>
#include "winextras.h"

#endif

static MouseHelper mouseHelperObj;

#ifdef Q_OS_UNIX
    static void finalSpringEvent(Display *display, unsigned int xmovecoor, unsigned int ymovecoor)
    {
        Q_UNUSED(display);
        Q_UNUSED(xmovecoor);
        Q_UNUSED(ymovecoor);
    }

#elif defined(Q_OS_WIN)
    static void finalSpringEvent(INPUT *event, unsigned int xmovecoor, unsigned int ymovecoor, unsigned int width, unsigned int height)
    {
        int fx = ceil(xmovecoor * (65535.0/(double)width));
        int fy = ceil(ymovecoor * (65535.0/(double)height));
        event[0].mi.dx = fx;
        event[0].mi.dy = fy;
        SendInput(1, event, sizeof(INPUT));
    }
#endif

// Create the event used by the operating system.
void sendevent(JoyButtonSlot *slot, bool pressed)
{
    //int code = slot->getSlotCode();
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();

    if (device == JoyButtonSlot::JoyKeyboard)
    {
        EventHandlerFactory::getInstance()->handler()->sendKeyboardEvent(slot, pressed);
    }
    else if (device == JoyButtonSlot::JoyMouseButton)
    {
        EventHandlerFactory::getInstance()->handler()->sendMouseButtonEvent(slot, pressed);
    }
    else if (device == JoyButtonSlot::JoyTextEntry && pressed && !slot->getTextData().isEmpty())
    {
        EventHandlerFactory::getInstance()->handler()->sendTextEntryEvent(slot->getTextData());
    }
    else if (device == JoyButtonSlot::JoyExecute && pressed && !slot->getTextData().isEmpty())
    {
        QString execString = slot->getTextData();
        QProcess::startDetached(execString);
        //if (pressed)
        //{
            /*QProcess proctor;
            proctor.setProgram("gvim");
            proctor.start(QIODevice::ReadOnly);
            */
            //QProcess::startDetached("gvim");
        //}
    }
}

void sendevent(int code1, int code2)
{
    EventHandlerFactory::getInstance()->handler()->sendMouseEvent(code1, code2);
}

void sendSpringEvent(PadderCommon::springModeInfo *fullSpring, PadderCommon::springModeInfo *relativeSpring, int* const mousePosX, int* const mousePosY)
{
    mouseHelperObj.mouseTimer.stop();

    if ((fullSpring->displacementX >= -2.0 && fullSpring->displacementX <= 1.0 &&
        fullSpring->displacementY >= -2.0 && fullSpring->displacementY <= 1.0) ||
        (relativeSpring && (relativeSpring->displacementX >= -2.0 && relativeSpring->displacementX <= 1.0 &&
        relativeSpring->displacementY >= -2.0 && relativeSpring->displacementY <= 1.0)))
    {
        int xmovecoor = 0;
        int ymovecoor = 0;
        int width = 0;
        int height = 0;
        int midwidth = 0;
        int midheight = 0;
        int destSpringWidth = 0;
        int destSpringHeight = 0;
        int destMidWidth = 0;
        int destMidHeight = 0;
        int currentMouseX = 0;
        int currentMouseY = 0;

        QDesktopWidget deskWid;
        if (fullSpring->screen >= deskWid.screenCount())
        {
            fullSpring->screen = -1;
        }

        QRect deskRect = deskWid.screenGeometry(fullSpring->screen);
//#if defined (Q_OS_UNIX)
        width = deskRect.width();
        height = deskRect.height();
        QPoint currentPoint = QCursor::pos();
        currentMouseX = currentPoint.x();
        currentMouseY = currentPoint.y();

        //qDebug() << "Current Mouse X: " << currentMouseX;
        //qDebug() << "Current Mouse Y: " << currentMouseY;
        //qDebug() << "WIDTH: " << width;
        //qDebug() << "HEIGHT: " << height;

//#elif defined (Q_OS_WIN)
/*
        POINT cursorPoint;
        GetCursorPos(&cursorPoint);

        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
        currentMouseX = cursorPoint.x;
        currentMouseY = cursorPoint.y;
*/
//#endif

        midwidth = width / 2;
        midheight = height / 2;

        int springWidth = fullSpring->width;
        int springHeight = fullSpring->height;

        if (springWidth >= 2 && springHeight >= 2)
        {
            destSpringWidth = qMin(springWidth, width);
            destSpringHeight = qMin(springHeight, height);
        }
        else
        {
            destSpringWidth = width;
            destSpringHeight = height;
        }

        destMidWidth = destSpringWidth / 2;
        destMidHeight = destSpringHeight / 2;

        unsigned int pivotX = currentMouseX;
        unsigned int pivotY = currentMouseY;
        if (relativeSpring)
        {
            if (mouseHelperObj.pivotPoint[0] != -1)
            {
                pivotX = mouseHelperObj.pivotPoint[0];
            }
            else
            {
                pivotX = currentMouseX;
            }

            if (mouseHelperObj.pivotPoint[1] != -1)
            {
                pivotY = mouseHelperObj.pivotPoint[1];
            }
            else
            {
                pivotY = currentMouseY;
            }
        }

        xmovecoor = (fullSpring->displacementX >= -1.0) ? (midwidth + (fullSpring->displacementX * destMidWidth)): pivotX;
        ymovecoor = (fullSpring->displacementY >= -1.0) ? (midheight + (fullSpring->displacementY * destMidHeight)) : pivotY;

        // Add top left coordinates of screen to new cursor coordinates.
        xmovecoor += deskRect.x();
        ymovecoor += deskRect.y();

        unsigned int fullSpringDestX = xmovecoor;
        unsigned int fullSpringDestY = ymovecoor;

        unsigned int destRelativeWidth = 0;
        unsigned int destRelativeHeight = 0;
        if (relativeSpring && relativeSpring->width >= 2 && relativeSpring->height >= 2)
        {
            destRelativeWidth = relativeSpring->width;
            destRelativeHeight = relativeSpring->height;

            int xRelativeMoovCoor = 0;
            if (relativeSpring->displacementX >= -1.0)
            {
                xRelativeMoovCoor = (relativeSpring->displacementX * destRelativeWidth) / 2;
            }

            int yRelativeMoovCoor = 0;
            if (relativeSpring->displacementY >= -1.0)
            {
                yRelativeMoovCoor = (relativeSpring->displacementY * destRelativeHeight) / 2;
            }

            xmovecoor += xRelativeMoovCoor;
            ymovecoor += yRelativeMoovCoor;
        }

        //qDebug() << "XMOVECOOR: " << xmovecoor;
        //qDebug() << "YMOVECOOR: " << ymovecoor;

        if (mousePosX)
        {
            *mousePosX = xmovecoor;
        }

        if (mousePosY)
        {
            *mousePosY = ymovecoor;
        }

        if (xmovecoor != currentMouseX || ymovecoor != currentMouseY)
        {
            double diffx = abs(currentMouseX - xmovecoor);
            double diffy = abs(currentMouseY - ymovecoor);

#ifdef Q_OS_WIN
            INPUT temp[1] = {};
            temp[0].type = INPUT_MOUSE;
            temp[0].mi.mouseData = 0;
            temp[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

#endif

            //qDebug() << "MIDX: " << (deskRect.x() + midwidth);
            //qDebug() << "MIDY: " << (deskRect.y() + midheight);

            // If either position is set to center, force update.
            if (xmovecoor == (deskRect.x() + midwidth) || ymovecoor == (deskRect.y() + midheight))
            {
#if defined(Q_OS_UNIX)
                EventHandlerFactory::getInstance()->handler()->sendMouseEvent(xmovecoor - currentMouseX,
                                                                              ymovecoor - currentMouseY);
#elif defined(Q_OS_WIN)
                if (fullSpring->screen <= -1)
                {
                    finalSpringEvent(temp, xmovecoor, ymovecoor, width, height);
                }
                else
                {
                    sendevent(xmovecoor - currentMouseX, ymovecoor - currentMouseY);
                }
#endif

            }
            else if (!mouseHelperObj.springMouseMoving && relativeSpring &&
                (relativeSpring->displacementX >= -1.0 || relativeSpring->displacementY >= -1.0) &&
                (diffx >= destRelativeWidth*.013 || diffy >= destRelativeHeight*.013))
            {
                mouseHelperObj.springMouseMoving = true;
#if defined(Q_OS_UNIX)
                EventHandlerFactory::getInstance()->handler()->sendMouseEvent(xmovecoor - currentMouseX,
                                                                              ymovecoor - currentMouseY);

#elif defined(Q_OS_WIN)
                if (fullSpring->screen <= -1)
                {
                    finalSpringEvent(temp, xmovecoor, ymovecoor, width, height);
                }
                else
                {
                    sendevent(xmovecoor - currentMouseX, ymovecoor - currentMouseY);
                }
#endif
                mouseHelperObj.mouseTimer.start(11);
            }
            else if (!mouseHelperObj.springMouseMoving && (diffx >= destSpringWidth*.013 || diffy >= destSpringHeight*.013))
            {
                mouseHelperObj.springMouseMoving = true;
#if defined(Q_OS_UNIX)

                EventHandlerFactory::getInstance()->handler()->sendMouseEvent(xmovecoor - currentMouseX,
                                                                              ymovecoor - currentMouseY);

#elif defined(Q_OS_WIN)
                if (fullSpring->screen <= -1)
                {
                    finalSpringEvent(temp, xmovecoor, ymovecoor, width, height);
                }
                else
                {
                    sendevent(xmovecoor - currentMouseX, ymovecoor - currentMouseY);
                }
#endif

                //qDebug() << QTime::currentTime();
                //qDebug() << "X: " << (xmovecoor - currentMouseX / (deskRect.x() + midheight));
                //qDebug() << "Y: " << ymovecoor - currentMouseY / (deskRect.y() + midheight);

                mouseHelperObj.mouseTimer.start(11);
            }

            else if (mouseHelperObj.springMouseMoving && (diffx < 2 && diffy < 2))
            {
                mouseHelperObj.springMouseMoving = false;
            }

            else if (mouseHelperObj.springMouseMoving)
            {
#if defined(Q_OS_UNIX)
                EventHandlerFactory::getInstance()->handler()->sendMouseEvent(xmovecoor - currentMouseX,
                                                                              ymovecoor - currentMouseY);

#elif defined(Q_OS_WIN)
                if (fullSpring->screen <= -1)
                {
                    finalSpringEvent(temp, xmovecoor, ymovecoor, width, height);
                }
                else
                {
                    sendevent(xmovecoor - currentMouseX, ymovecoor - currentMouseY);
                }
#endif

                mouseHelperObj.mouseTimer.start(11);
            }


            mouseHelperObj.previousCursorLocation[0] = currentMouseX;
            mouseHelperObj.previousCursorLocation[1] = currentMouseY;
            mouseHelperObj.pivotPoint[0] = fullSpringDestX;
            mouseHelperObj.pivotPoint[1] = fullSpringDestY;
        }
        else if (mouseHelperObj.previousCursorLocation[0] == xmovecoor &&
                 mouseHelperObj.previousCursorLocation[1] == ymovecoor)
        {
            mouseHelperObj.springMouseMoving = false;
        }
        else
        {
            mouseHelperObj.previousCursorLocation[0] = currentMouseX;
            mouseHelperObj.previousCursorLocation[1] = currentMouseY;
            mouseHelperObj.pivotPoint[0] = fullSpringDestX;
            mouseHelperObj.pivotPoint[1] = fullSpringDestY;

            mouseHelperObj.mouseTimer.start(11);
        }
    }
    else
    {
        mouseHelperObj.springMouseMoving = false;
        mouseHelperObj.pivotPoint[0] = -1;
        mouseHelperObj.pivotPoint[1] = -1;
    }
}

int X11KeySymToKeycode(QString key)
{
    int tempcode = 0;
#if defined (Q_OS_UNIX)

    BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

    if (key.length() > 0)
    {
#ifdef WITH_XTEST
        if (handler->getIdentifier() == "xtest")
        {
            Display* display = X11Extras::getInstance()->display();
            tempcode = XKeysymToKeycode(display, XStringToKeysym(key.toUtf8().data()));
        }
#endif

#ifdef WITH_UINPUT
        if (handler->getIdentifier() == "uinput")
        {
            tempcode = UInputHelper::getInstance()->getVirtualKey(key);
        }
#endif
    }

#elif defined (Q_OS_WIN)
    if (key.length() > 0)
    {
        tempcode = WinExtras::getVirtualKey(key);
        if (tempcode <= 0 && key.length() == 1)
        {
            //qDebug() << "KEY: " << key;
            //int oridnal = key.toUtf8().constData()[0];
            int ordinal = QVariant(key.toUtf8().constData()[0]).toInt();
            tempcode = VkKeyScan(ordinal);
            int modifiers = tempcode >> 8;
            tempcode = tempcode & 0xff;
            if ((modifiers & 1) != 0) tempcode |= VK_SHIFT;
            if ((modifiers & 2) != 0) tempcode |= VK_CONTROL;
            if ((modifiers & 4) != 0) tempcode |= VK_MENU;
            //tempcode = VkKeyScan(QVariant(key.constData()).toInt());
            //tempcode = OemKeyScan(key.toUtf8().toInt());
            //tempcode = OemKeyScan(ordinal);
        }
    }

#endif

    return tempcode;
}

QString keycodeToKeyString(int keycode, unsigned int alias)
{
    QString newkey;

#if defined (Q_OS_UNIX)
    Q_UNUSED(alias);

    if (keycode <= 0)
    {
        newkey = "[NO KEY]";
    }
    else
    {
        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

#ifdef WITH_XTEST
        if (handler->getIdentifier() == "xtest")
        {
            Display* display = X11Extras::getInstance()->display();
            newkey = QString("0x%1").arg(keycode, 0, 16);
            QString tempkey = XKeysymToString(XkbKeycodeToKeysym(display, keycode, 0, 0));
            QString tempalias = X11Extras::getInstance()->getDisplayString(tempkey);
            if (!tempalias.isEmpty())
            {
                newkey = tempalias;
            }
            else
            {
                XKeyPressedEvent tempevent;
                tempevent.keycode = keycode;
                tempevent.type = KeyPress;
                tempevent.display = display;
                tempevent.state = 0;

                char tempstring[256];
                memset(tempstring, 0, sizeof(tempstring));
                int bitestoreturn = sizeof(tempstring) - 1;
                int numchars = XLookupString(&tempevent, tempstring, bitestoreturn, NULL, NULL);
                if (numchars > 0)
                {
                    tempstring[numchars] = '\0';
                    newkey = QString::fromUtf8(tempstring);
                    //qDebug() << "NEWKEY:" << newkey << endl;
                    //qDebug() << "NEWKEY LEGNTH:" << numchars << endl;
                }
                else
                {
                    newkey = tempkey;
                }
            }
        }
#endif

#ifdef WITH_UINPUT
        if (handler->getIdentifier() == "uinput")
        {
            QString tempalias = UInputHelper::getInstance()->getDisplayString(keycode);
            if (!tempalias.isEmpty())
            {
                newkey = tempalias;
            }
            else
            {
                newkey = QString("0x%1").arg(keycode, 0, 16);
            }
        }
#endif
    }

#elif defined (Q_OS_WIN)
    wchar_t buffer[50] = {0};

    QString tempalias = WinExtras::getDisplayString(keycode);
    if (!tempalias.isEmpty())
    {
        newkey = tempalias;
    }
    else
    {
        int scancode = WinExtras::scancodeFromVirtualKey(keycode, alias);

        if (keycode >= VK_BROWSER_BACK && keycode <= VK_LAUNCH_APP2)
        {
            newkey.append(QString("0x%1").arg(keycode, 0, 16));
        }
        else
        {
            int length = GetKeyNameTextW(scancode << 16, buffer, sizeof(buffer));
            if (length > 0)
            {
                newkey = QString::fromWCharArray(buffer);
            }
            else
            {
                newkey.append(QString("0x%1").arg(keycode, 0, 16));
            }
        }
    }

#endif

    return newkey;
}

unsigned int X11KeyCodeToX11KeySym(unsigned int keycode)
{
#ifdef Q_OS_WIN
    Q_UNUSED(keycode);
    return 0;
#else
    #ifdef WITH_X11
    Display* display = X11Extras::getInstance()->display();
    unsigned int tempcode = XkbKeycodeToKeysym(display, keycode, 0, 0);
    return tempcode;
    #else

    Q_UNUSED(keycode);
    return 0;
    #endif
#endif
}

QString keysymToKeyString(int keysym, unsigned int alias)
{
    QString newkey;

#if defined (Q_OS_UNIX)
    Q_UNUSED(alias);

    BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

    if (handler->getIdentifier() == "xtest")
    {
        Display* display = X11Extras::getInstance()->display();
        unsigned int keycode = 0;
        if (keysym > 0)
        {
            keycode = XKeysymToKeycode(display, keysym);
        }
        newkey = keycodeToKeyString(keycode);
    }
    else if (handler->getIdentifier() == "uinput")
    {
        newkey = keycodeToKeyString(keysym);
    }

#else
    newkey = keycodeToKeyString(keysym, alias);
#endif
    return newkey;
}
