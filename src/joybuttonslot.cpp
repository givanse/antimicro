//#include <QDebug>
#include <QFileInfo>

#include "inputdevice.h"
#include "joybuttonslot.h"

#include "antkeymapper.h"
#include "event.h"

//#ifdef Q_OS_WIN
//  #include "qtwinkeymapper.h"
//#endif

const int JoyButtonSlot::JOYSPEED = 20;
const QString JoyButtonSlot::xmlName = "slot";
const int JoyButtonSlot::MAXTEXTENTRYDISPLAYLENGTH = 40;

JoyButtonSlot::JoyButtonSlot(QObject *parent) :
    QObject(parent)
{
    deviceCode = 0;
    mode = JoyKeyboard;
    distance = 0.0;
    previousDistance = 0.0;
    qkeyaliasCode = 0;
    easingActive = false;
}

JoyButtonSlot::JoyButtonSlot(int code, JoySlotInputAction mode, QObject *parent) :
    QObject(parent)
{
    deviceCode = 0;
    qkeyaliasCode = 0;

    if (code > 0)
    {
        deviceCode = code;
    }

    this->mode = mode;
    distance = 0.0;
    easingActive = false;
}

JoyButtonSlot::JoyButtonSlot(int code, unsigned int alias, JoySlotInputAction mode, QObject *parent) :
    QObject(parent)
{
    deviceCode = 0;
    qkeyaliasCode = 0;

    if (code > 0)
    {
        deviceCode = code;
    }

    if (alias > 0)
    {
        qkeyaliasCode = alias;
    }

    this->mode = mode;
    distance = 0.0;
    easingActive = false;
}

JoyButtonSlot::JoyButtonSlot(JoyButtonSlot *slot, QObject *parent) :
    QObject(parent)
{
    deviceCode = slot->deviceCode;
    qkeyaliasCode = slot->qkeyaliasCode;
    mode = slot->mode;
    distance = slot->distance;
    easingActive = false;
    textData = slot->getTextData();
}

JoyButtonSlot::JoyButtonSlot(QString text, JoySlotInputAction mode, QObject *parent) :
    QObject(parent)
{
    deviceCode = 0;
    qkeyaliasCode = 0;
    this->mode = mode;
    distance = 0.0;
    easingActive = false;
    if (mode == JoyLoadProfile ||
        mode == JoyTextEntry ||
        mode == JoyExecute)
    {
        textData = text;
    }
}

void JoyButtonSlot::setSlotCode(int code)
{
    if (code >= 0)
    {
        deviceCode = code;
        qkeyaliasCode = 0;
    }
}

void JoyButtonSlot::setSlotCode(int code, unsigned int alias)
{
    if (mode == JoyButtonSlot::JoyKeyboard && code > 0)
    {
        deviceCode = code;
        qkeyaliasCode = alias;
    }
    else if (code >= 0)
    {
        deviceCode = code;
        qkeyaliasCode = 0;
    }
}

unsigned int JoyButtonSlot::getSlotCodeAlias()
{
    return qkeyaliasCode;
}

int JoyButtonSlot::getSlotCode()
{
    return deviceCode;
}

void JoyButtonSlot::setSlotMode(JoySlotInputAction selectedMode)
{
    mode = selectedMode;
}

JoyButtonSlot::JoySlotInputAction JoyButtonSlot::getSlotMode()
{
    return mode;
}

QString JoyButtonSlot::movementString()
{
    QString newlabel;

    if (mode == JoyMouseMovement)
    {
        newlabel.append(tr("Mouse")).append(" ");
        if (deviceCode == JoyButtonSlot::MouseUp)
        {
            newlabel.append(tr("Up"));
        }
        else if (deviceCode == JoyButtonSlot::MouseDown)
        {
            newlabel.append(tr("Down"));
        }
        else if (deviceCode == JoyButtonSlot::MouseLeft)
        {
            newlabel.append(tr("Left"));
        }
        else if (deviceCode == JoyButtonSlot::MouseRight)
        {
            newlabel.append(tr("Right"));
        }
    }

    return newlabel;
}

void JoyButtonSlot::setDistance(double distance)
{
    this->distance = distance;
}

double JoyButtonSlot::getMouseDistance()
{
    return distance;
}

QElapsedTimer* JoyButtonSlot::getMouseInterval()
{
    return &mouseInterval;
}

void JoyButtonSlot::restartMouseInterval()
{
    mouseInterval.restart();
}

void JoyButtonSlot::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == "slot")
    {
        QString profile;
        QString tempStringData;

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "slot"))
        {
            if (xml->name() == "code" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                bool ok = false;
                unsigned int tempchoice = temptext.toInt(&ok, 0);
                if (ok)
                {
                    this->setSlotCode(tempchoice);
                }
            }
            else if (xml->name() == "profile" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                profile = temptext;
            }
            else if (xml->name() == "text" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                tempStringData = temptext;
            }
            else if (xml->name() == "path" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                tempStringData = temptext;
            }
            else if (xml->name() == "mode" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();

                if (temptext == "keyboard")
                {
                    this->setSlotMode(JoyKeyboard);
                }
                else if (temptext == "mousebutton")
                {
                    this->setSlotMode(JoyMouseButton);
                }
                else if (temptext == "mousemovement")
                {
                    this->setSlotMode(JoyMouseMovement);
                }
                else if (temptext == "pause")
                {
                    this->setSlotMode(JoyPause);
                }
                else if (temptext == "hold")
                {
                    this->setSlotMode(JoyHold);
                }
                else if (temptext == "cycle")
                {
                    this->setSlotMode(JoyCycle);
                }
                else if (temptext == "distance")
                {
                    this->setSlotMode(JoyDistance);
                }
                else if (temptext == "release")
                {
                    this->setSlotMode(JoyRelease);
                }
                else if (temptext == "mousespeedmod")
                {
                    this->setSlotMode(JoyMouseSpeedMod);
                }
                else if (temptext == "keypress")
                {
                    this->setSlotMode(JoyKeyPress);
                }
                else if (temptext == "delay")
                {
                    this->setSlotMode(JoyDelay);
                }
                else if (temptext == "loadprofile")
                {
                    this->setSlotMode(JoyLoadProfile);
                }
                else if (temptext == "setchange")
                {
                    this->setSlotMode(JoySetChange);
                }
                else if (temptext == "textentry")
                {
                    this->setSlotMode(JoyTextEntry);
                }
                else if (temptext == "execute")
                {
                    this->setSlotMode(JoyExecute);
                }
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        if (this->getSlotMode() == JoyButtonSlot::JoyKeyboard)
        {
            unsigned int virtualkey = AntKeyMapper::getInstance()->returnVirtualKey(this->getSlotCode());
            unsigned int tempkey = this->getSlotCode();

            if (virtualkey)
            {
                // Mapping found a valid native keysym.
                this->setSlotCode(virtualkey, tempkey);
            }
            else if ((unsigned int)this->getSlotCode() > QtKeyMapperBase::nativeKeyPrefix)
            {
                // Value is in the native key range. Remove prefix and use
                // new value as a native keysym.
                unsigned int temp = this->getSlotCode() - QtKeyMapperBase::nativeKeyPrefix;
                this->setSlotCode(temp);
            }
        }
        else if (this->getSlotMode() == JoyButtonSlot::JoyLoadProfile && !profile.isEmpty())
        {
            QFileInfo profileInfo(profile);
            if (!profileInfo.exists() || !(profileInfo.suffix() == "amgp" || profileInfo.suffix() == "xml"))
            {
                this->setTextData("");
            }
            else
            {
                this->setTextData(profile);
            }
        }
        else if (this->getSlotMode() == JoySetChange)
        {
            if (!this->getSlotCode() >= 0 && !this->getSlotCode() < InputDevice::NUMBER_JOYSETS)
            {
                this->setSlotCode(-1);
            }
        }
        else if (this->getSlotMode() == JoyTextEntry && !tempStringData.isEmpty())
        {
            this->setTextData(tempStringData);
        }
        else if (this->getSlotMode() == JoyExecute && !tempStringData.isEmpty())
        {
            QFileInfo tempFile(tempStringData);
            if (tempFile.exists() && tempFile.isExecutable())
            {
                this->setTextData(tempStringData);
            }
        }
    }
}

void JoyButtonSlot::writeConfig(QXmlStreamWriter *xml)
{
    xml->writeStartElement(getXmlName());

    if (mode == JoyKeyboard)
    {
        unsigned int basekey = AntKeyMapper::getInstance()->returnQtKey(deviceCode);
        unsigned int qtkey = this->getSlotCodeAlias();
        if (qtkey > 0 || basekey > 0)
        {
            // Did not add an alias to slot. If a possible Qt key value
            // was found, use it.
            if (qtkey == 0 && basekey > 0)
            {
                qtkey = basekey;
            }

            // Found a valid abstract keysym.
            //qDebug() << "ANT KEY: " << QString::number(qtkey, 16);
            xml->writeTextElement("code", QString("0x%1").arg(qtkey, 0, 16));
        }
        else if (deviceCode > 0)
        {
            // No abstraction provided for key. Add prefix to native keysym.
            unsigned int tempkey = deviceCode | QtKeyMapperBase::nativeKeyPrefix;
            //qDebug() << "ANT KEY: " << QString::number(tempkey, 16);
            xml->writeTextElement("code", QString("0x%1").arg(tempkey, 0, 16));
        }
    }
    else if (mode == JoyLoadProfile && !textData.isEmpty())
    {
        xml->writeTextElement("profile", textData);
    }
    else if (mode == JoyTextEntry && !textData.isEmpty())
    {
        xml->writeTextElement("text", textData);
    }
    else if (mode == JoyExecute && !textData.isEmpty())
    {
        xml->writeTextElement("path", textData);
    }
    else
    {
        xml->writeTextElement("code", QString::number(deviceCode));
    }

    xml->writeStartElement("mode");
    if (mode == JoyKeyboard)
    {
        xml->writeCharacters("keyboard");
    }
    else if (mode == JoyMouseButton)
    {
        xml->writeCharacters("mousebutton");
    }
    else if (mode == JoyMouseMovement)
    {
        xml->writeCharacters("mousemovement");
    }
    else if (mode == JoyPause)
    {
        xml->writeCharacters("pause");
    }
    else if (mode == JoyHold)
    {
        xml->writeCharacters("hold");
    }
    else if (mode == JoyCycle)
    {
        xml->writeCharacters("cycle");
    }
    else if (mode == JoyDistance)
    {
        xml->writeCharacters("distance");
    }
    else if (mode == JoyRelease)
    {
        xml->writeCharacters("release");
    }
    else if (mode == JoyMouseSpeedMod)
    {
        xml->writeCharacters("mousespeedmod");
    }
    else if (mode == JoyKeyPress)
    {
        xml->writeCharacters("keypress");
    }
    else if (mode == JoyDelay)
    {
        xml->writeCharacters("delay");
    }
    else if (mode == JoyLoadProfile)
    {
        xml->writeCharacters("loadprofile");
    }
    else if (mode == JoySetChange)
    {
        xml->writeCharacters("setchange");
    }
    else if (mode == JoyTextEntry)
    {
        xml->writeCharacters("textentry");
    }
    else if (mode == JoyExecute)
    {
        xml->writeCharacters("execute");
    }

    xml->writeEndElement();

    xml->writeEndElement();
}

QString JoyButtonSlot::getXmlName()
{
    return this->xmlName;
}

QString JoyButtonSlot::getSlotString()
{
    QString newlabel;

    if (deviceCode >= 0)
    {
        if (mode == JoyButtonSlot::JoyKeyboard)
        {
            unsigned int tempDeviceCode = deviceCode;
#ifdef Q_OS_WIN
            //static QtWinKeyMapper nativeWinKeyMapper;
            QtKeyMapperBase *nativeWinKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
            if (nativeWinKeyMapper)
            {
                tempDeviceCode = nativeWinKeyMapper->returnVirtualKey(qkeyaliasCode);
            }
#endif
            newlabel = newlabel.append(keysymToKeyString(tempDeviceCode, qkeyaliasCode).toUpper());
            //newlabel = newlabel.append(keysymToKeyString(deviceCode, qkeyaliasCode).toUpper());
        }
        else if (mode == JoyButtonSlot::JoyMouseButton)
        {
            newlabel.append(tr("Mouse")).append(" ");
            switch (deviceCode)
            {
                case 1:
                    newlabel.append(tr("LB"));
                    break;
                case 2:
                    newlabel.append(tr("MB"));
                    break;
                case 3:
                    newlabel.append(tr("RB"));
                    break;
#ifdef Q_OS_WIN
                case 8:
                    newlabel.append(tr("B4"));
                    break;
                case 9:
                    newlabel.append(tr("B5"));
                    break;
#endif
                default:
                    newlabel.append(QString::number(deviceCode));
                    break;
            }
        }
        else if (mode == JoyMouseMovement)
        {
            newlabel.append(movementString());
        }
        else if (mode == JoyPause)
        {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            newlabel.append(tr("Pause")).append(" ");
            if (minutes > 0)
            {
                newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
            }

            newlabel.append(QString("%1.%2")
                    .arg(seconds, 2, 10, QChar('0'))
                    .arg(hundredths, 2, 10, QChar('0')));
        }
        else if (mode == JoyHold)
        {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            newlabel.append(tr("Hold")).append(" ");
            if (minutes > 0)
            {
                newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
            }

            newlabel.append(QString("%1.%2")
                    .arg(seconds, 2, 10, QChar('0'))
                    .arg(hundredths, 2, 10, QChar('0')));
        }
        else if (mode == JoyButtonSlot::JoyCycle)
        {
            newlabel.append(tr("Cycle"));
        }
        else if (mode == JoyDistance)
        {
            QString temp(tr("Distance"));
            temp.append(" ").append(QString::number(deviceCode).append("%"));
            newlabel.append(temp);
        }
        else if (mode == JoyRelease)
        {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            newlabel.append(tr("Release")).append(" ");
            if (minutes > 0)
            {
                newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
            }

            newlabel.append(QString("%1.%2")
                    .arg(seconds, 2, 10, QChar('0'))
                    .arg(hundredths, 2, 10, QChar('0')));
        }
        else if (mode == JoyMouseSpeedMod)
        {
            QString temp;
            temp.append(tr("Mouse Mod")).append(" ");
            temp.append(QString::number(deviceCode).append("%"));
            newlabel.append(temp);
        }
        else if (mode == JoyKeyPress)
        {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            QString temp;
            temp.append(tr("Press Time")).append(" ");
            if (minutes > 0)
            {
                temp.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
            }

            temp.append(QString("%1.%2")
                    .arg(seconds, 2, 10, QChar('0'))
                    .arg(hundredths, 2, 10, QChar('0')));

            newlabel.append(temp);
        }
        else if (mode == JoyDelay)
        {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            QString temp;
            temp.append(tr("Delay")).append(" ");
            if (minutes > 0)
            {
                temp.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
            }

            temp.append(QString("%1.%2")
                    .arg(seconds, 2, 10, QChar('0'))
                    .arg(hundredths, 2, 10, QChar('0')));

            newlabel.append(temp);
        }
        else if (mode == JoyLoadProfile)
        {
            if (!textData.isEmpty())
            {
                QFileInfo profileInfo(textData);
                QString temp;
                temp.append(tr("Load %1").arg(profileInfo.baseName()));
                newlabel.append(temp);
            }
        }
        else if (mode == JoySetChange)
        {
            newlabel.append(tr("Set Change %1").arg(deviceCode+1));
        }
        else if (mode == JoyTextEntry)
        {
            QString temp = textData;
            if (temp.length() > MAXTEXTENTRYDISPLAYLENGTH)
            {
                temp.truncate(MAXTEXTENTRYDISPLAYLENGTH - 3);
                temp.append("...");
            }
            newlabel.append(tr("[Text] %1").arg(temp));
        }
        else if (mode == JoyExecute)
        {
            QString temp;
            if (!textData.isEmpty())
            {
                QFileInfo tempFileInfo(textData);
                temp.append(tempFileInfo.fileName());
            }

            newlabel.append(tr("[Exec] %1").arg(temp));
        }
    }
    else
    {
        newlabel = newlabel.append(tr("[NO KEY]"));
    }

    return newlabel;
}

void JoyButtonSlot::setPreviousDistance(double distance)
{
    previousDistance = distance;
}

double JoyButtonSlot::getPreviousDistance()
{
    return previousDistance;
}

bool JoyButtonSlot::isModifierKey()
{
    bool modifier = false;
    if (mode == JoyKeyboard && AntKeyMapper::getInstance()->isModifierKey(qkeyaliasCode))
    {
        modifier = true;
    }

    return modifier;
}

bool JoyButtonSlot::isEasingActive()
{
    return easingActive;
}

void JoyButtonSlot::setEasingStatus(bool isActive)
{
    easingActive = isActive;
}

QTime* JoyButtonSlot::getEasingTime()
{
    return &easingTime;
}

void JoyButtonSlot::setTextData(QString textData)
{
    this->textData = textData;
}

QString JoyButtonSlot::getTextData()
{
    return textData;
}

bool JoyButtonSlot::isValidSlot()
{
    bool result = true;
    switch (mode)
    {
        case JoyLoadProfile:
        case JoyTextEntry:
        case JoyExecute:
        {
            if (textData.isEmpty())
            {
                result = false;
            }

            break;
        }
        case JoySetChange:
        {
            if (deviceCode < 0)
            {
                result = false;
            }

            break;
        }
    }

    return result;
}
