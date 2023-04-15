#include "arm.h"

Arm::Arm(QObject *parent) : QObject(parent)
{
    frame.QByteArray::clear();
    containerX = 0;
    containerY = 0;
    containerZ = 0;
    containerPower = 0;
    activeButtonFunction = ButtonFunction::none;
    buttonPressed = 120;

    joyX = 0.00;
    joyY = 0.00;
    joyZ = 0.00;

    powerOnMotors = 0;

    deadzone = 0.25;

    motorX = 0;
    motorY = 0;
    motorZ = 0;
}

bool Arm::calculateSegmentsSpeeds(const int x, const int y, const int z, const int power, ButtonFunction buttonFunction, int buttonPressed)
{
    frame.QByteArray::clear();

    int16_t motorBase = 11392;
    int16_t motorSegment2Middle = 11392;
    int16_t motorSegment1Bottom = 11392;
    int16_t motorJawsClench = 11392;
    int16_t motorJawsRotation = 11392;
    int16_t motorJawsPosition = 11392;

    joyX = (x - 32767) / 32767.0f;
    joyY = (y - 32767) / 32767.0f;
    joyZ = (z - 32767) / 32767.0f;

    powerOnMotors = 100 * power;

    if(!(joyX >= -deadzone && joyX <= deadzone && joyY >= -deadzone && joyY <= deadzone))
    {
        motorX = joyX*powerOnMotors;
        motorY = joyY*powerOnMotors;
        if(!(joyZ >= -deadzone && joyZ <= deadzone))
        {
            motorZ = joyZ*powerOnMotors;
        }
        else
        {
            motorZ = 0;
        }
    }
    else
    {
        motorX = 0;
        motorY = 0;
        if(!(joyZ >= -deadzone && joyZ <= deadzone))
        {
             motorZ = joyZ*powerOnMotors;
        }
        else
        {
            motorZ = 0;
        }
    }


    switch(buttonFunction)
    {
    case none:
        break;
    case first:
        motorBase = motorZ + 11392;
        motorSegment1Bottom = motorY + 11392;
        //motorJawsRotation = motorX+11392;
        break;
    case second:
        motorBase = motorZ + 11392;
        motorSegment2Middle = -motorY+11392;
        //motorJawsRotation = -motorX+11392;
        break;
    case third:
        motorBase = motorZ + 11392;
        motorJawsPosition = motorY+11392;
        motorJawsRotation = -motorX+11392;
        break;
    case jaws:
        motorBase = motorZ + 11392;
        motorJawsRotation = -motorX+11392;
        motorJawsClench = 2*motorY+11392;
        break;
    case all:
        motorBase = motorZ + 11392;
        switch(buttonPressed)
        {
        case 0:
            motorJawsPosition = motorY+11392;
            motorJawsClench = 11392;
            motorSegment2Middle = 11392;
            motorSegment1Bottom = 11392;
            break;
        case 1:
            motorSegment1Bottom = motorY+11392;
            motorJawsPosition = 11392;
            motorJawsClench = 11392;
            motorSegment2Middle = 11392;
            break;
        case 2:
            motorSegment2Middle = motorY+11392;
            motorSegment1Bottom = 11392;
            motorJawsPosition = 11392;
            motorJawsClench = 11392;
            break;
        case 3:
            motorJawsClench = 10*motorY+11392;
            motorSegment2Middle = 11392;
            motorSegment1Bottom = 11392;
            motorJawsPosition = 11392;
            break;
        default:
            motorSegment1Bottom = motorY+11392;
            motorSegment2Middle = -motorY+11392;
            motorJawsPosition = 11392;
            motorJawsClench = 11392;
            break;
        }
        motorJawsRotation = -motorX+11392;
        break;
    default:
        break;
    }

    if(motorJawsClench>22784) motorJawsClench = 22784;
    if(motorJawsClench<-22784) motorJawsClench = -22784;

    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream << motorBase << motorSegment2Middle << motorSegment1Bottom <<motorJawsClench << motorJawsRotation << motorJawsPosition;

    qDebug() << motorBase << motorSegment2Middle << motorSegment1Bottom <<motorJawsClench << motorJawsRotation << motorJawsPosition;


    return true;
}

void Arm::printButtonFunction(ButtonFunction buttonFunction)
{
    switch(buttonFunction)
    {
        case none:
            qDebug() << "Button function: none";
            break;
        case first:
            qDebug() << "Button function: first";
            break;
        case second:
            qDebug() << "Button function: second";
            break;
        case third:
            qDebug() << "Button function: third";
            break;
        case jaws:
            qDebug() << "Button function: jaws";
            break;
        case all:
            qDebug() << "Button function: all";
            break;
        case inverseKinematics:
            qDebug() << "Button function: inverseKinematics";
            break;
        default:
            qDebug() << "Unknown button function";
            break;
    }
}


