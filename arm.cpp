#include "arm.h"

Arm::Arm(QObject *parent) : QObject(parent)
{
    frame.QByteArray::clear();
    framePoker.QByteArray::clear();
    containerX = joystickCenter;
    containerY = joystickCenter;
    containerZ = joystickCenter;
    containerPower = 0;
    activeButtonFunction = ButtonFunction::None;
    buttonPressed = 120;

    joyX = joyY = joyZ = 0.0;
    powerOnMotors = 0;
    deadzone = 0.25;
    motorX = motorY = motorZ = 0;

    pokerAngle = 0;
    pokerPoke = 0;

    QDataStream stream(&framePoker, QIODevice::WriteOnly);
    stream << pokerAngle << pokerPoke;
}

bool Arm::calculateSegmentsSpeeds(const int x, const int y, const int z, const int power, ButtonFunction buttonFunction, int buttonPressed)
{
    frame.clear();

    constexpr int16_t motorInitial = 3200;

    int16_t motorBase = motorInitial;
    int16_t motorSegment2Middle = motorInitial;
    int16_t motorSegment1Bottom = motorInitial;
    int16_t motorJawsClench = motorInitial;
    int16_t motorJawsRotation = motorInitial;
    int16_t motorJawsPosition = motorInitial;

    joyX = (x - joystickCenter) / static_cast<float>(joystickCenter);
    joyY = (y - joystickCenter) / static_cast<float>(joystickCenter);
    joyZ = (z - joystickCenter) / static_cast<float>(joystickCenter);

    powerOnMotors = 32 * power;

    bool outsideDeadzoneX = joyX < -deadzone || joyX > deadzone;
    bool outsideDeadzoneY = joyY < -deadzone || joyY > deadzone;
    bool outsideDeadzoneZ = joyZ < -deadzone || joyZ > deadzone;

    motorX = outsideDeadzoneX ? joyX * powerOnMotors : 0;
    motorY = outsideDeadzoneY ? joyY * powerOnMotors : 0;
    motorZ = outsideDeadzoneZ ? joyZ * powerOnMotors : 0;

    switch(buttonFunction)
    {
    case None:
        break;
    case First:
        motorSegment1Bottom = motorY+3200;
        motorJawsPosition = 3200;
        motorJawsClench = 3200;
        motorSegment2Middle = 3200;
        break;
    case Second:
        motorSegment2Middle = motorY+3200;
        motorSegment1Bottom = 3200;
        motorJawsPosition = 3200;
        motorJawsClench = 3200;
        break;
    case Third:
        motorJawsPosition = motorY+3200;
        motorJawsClench = 3200;
        motorSegment2Middle = 3200;
        motorSegment1Bottom = 3200;
        break;
    case Jaws:
        if(motorY > 10) motorJawsClench = 6400;
        else if(motorY < -10) motorJawsClench = 0;
        else motorJawsClench = 3200;
        motorSegment2Middle = 3200;
        motorSegment1Bottom = 3200;
        motorJawsPosition = 3200;
        motorJawsRotation = -motorX+3200;
        break;
    case All:
        motorBase = motorZ + 3200;
        switch(buttonPressed)
        {
        case 0:
            motorJawsPosition = motorY+3200;
            motorJawsClench = 3200;
            motorSegment2Middle = 3200;
            motorSegment1Bottom = 3200;
            break;
        case 1:
            motorSegment1Bottom = motorY+3200;
            motorJawsPosition = 3200;
            motorJawsClench = 3200;
            motorSegment2Middle = 3200;
            break;
        case 2:
            motorSegment2Middle = -motorY+3200;
            motorSegment1Bottom = 3200;
            motorJawsPosition = 3200;
            motorJawsClench = 3200;
            break;
        case 3:
            if(motorY > 10) motorJawsClench = 6400;
            else if(motorY < -10) motorJawsClench = 0;
            else motorJawsClench = 3200;
            motorSegment2Middle = 3200;
            motorSegment1Bottom = 3200;
            motorJawsPosition = 3200;
            break;
        default:
            motorSegment1Bottom = motorY+3200;
             motorSegment2Middle = -motorY+3200;
            motorJawsPosition = 3200;
            motorJawsClench = 3200;
            break;
        }
        motorJawsRotation = -motorX+3200;
        break;
    default:
        break;
    }

    if(motorJawsClench>6400) motorJawsClench = 6400;
    if(motorJawsClench<0) motorJawsClench = 0;

    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream << motorBase << motorSegment2Middle << motorSegment1Bottom <<motorJawsClench << motorJawsRotation << motorJawsPosition;

    //qDebug() << x << y << z << power << buttonFunction;
    //qDebug() << motorBase << motorSegment2Middle << motorSegment1Bottom <<motorJawsClench << motorJawsRotation << motorJawsPosition;

    // Emit signals if the new value is different from the previous value
       if (motorBase != prevMotorBase) {
           emit motorBaseChanged(motorBase);
           prevMotorBase = motorBase;
       }
       if (motorSegment1Bottom != prevMotorSegment1Bottom) {
           emit motorFirstChanged(motorSegment1Bottom);
           prevMotorSegment1Bottom = motorSegment1Bottom;
       }
       if (motorSegment2Middle != prevMotorSegment2Middle) {
           emit motorSecondChanged(motorSegment2Middle);
           prevMotorSegment2Middle = motorSegment2Middle;
       }
       if (motorJawsPosition != prevMotorJawsPosition) {
           emit motorThirdChanged(motorJawsPosition);
           prevMotorJawsPosition = motorJawsPosition;
       }
       if (motorJawsRotation != prevMotorJawsRotation) {
           emit motorJawsChanged(motorJawsRotation);
           prevMotorJawsRotation = motorJawsRotation;
       }
       if (motorJawsClench != prevMotorJawsClench) {
           emit motorJawsClenchChanged(motorJawsClench);
           prevMotorJawsClench = motorJawsClench;
       }

       //qDebug() << framePoker;

    return true;
}

void Arm::processButtonPressed(int buttonPressedNow)
{
    buttonPressed = buttonPressedNow;

    for (int buttonFunctionKeyNumber = 0; buttonFunctionKeyNumber < 6; buttonFunctionKeyNumber++)
    {
        if (buttonPressedNow >= 4 && buttonPressedNow <= 9)
        {
            if (buttonPressedNow - 4 == buttonFunctionKeyNumber)
            {
                buttonFunctionKeys[buttonFunctionKeyNumber].isActive = true;
            }
            else
            {
                buttonFunctionKeys[buttonFunctionKeyNumber].isActive = false;
            }
            emit buttonFunctionChanged(buttonFunctionKeyNumber);
        }
    }
}
void Arm::onButtonFunctionIndexChanged(int index, int newIndex) {
    // Update the button function
    buttonFunctionKeys[index].function = static_cast<ButtonFunction>(newIndex);

    // Emit the signal
    emit buttonFunctionChanged(index);
}

void Arm::calculatePokerValues()
{
    QDataStream stream(&framePoker, QIODevice::WriteOnly);
    stream << pokerAngle << pokerPoke;
}
