#include "arm.h"

Arm::Arm(QObject *parent) : QObject(parent)
{
    frame.QByteArray::clear();
    containerX = 0;
    containerY = 0;
    containerZ = 0;
    containerPower = 0;
    activeButtonFunction = ButtonFunction::None;
    buttonPressed = 120;

    joyX = joyY = joyZ = 0.0;
    powerOnMotors = 0;
    deadzone = 0.25;
    motorX = motorY = motorZ = 0;
}

bool Arm::calculateSegmentsSpeeds(const int x, const int y, const int z, const int power, ButtonFunction buttonFunction, int buttonPressed)
{
    frame.clear();

    constexpr int16_t motorInitial = 11392;

    int16_t motorBase = motorInitial;
    int16_t motorSegment2Middle = motorInitial;
    int16_t motorSegment1Bottom = motorInitial;
    int16_t motorJawsClench = motorInitial;
    int16_t motorJawsRotation = motorInitial;
    int16_t motorJawsPosition = motorInitial;

    constexpr int32_t joystickCenter = 32767;
    joyX = (x - joystickCenter) / static_cast<float>(joystickCenter);
    joyY = (y - joystickCenter) / static_cast<float>(joystickCenter);
    joyZ = (z - joystickCenter) / static_cast<float>(joystickCenter);

    powerOnMotors = 100 * power;

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
        motorBase = motorZ + 11392;
        motorSegment1Bottom = motorY + 11392;
        //motorJawsRotation = motorX+11392;
        break;
    case Second:
        motorBase = motorZ + 11392;
        motorSegment2Middle = -motorY+11392;
        //motorJawsRotation = -motorX+11392;
        break;
    case Third:
        motorBase = motorZ + 11392;
        motorJawsPosition = motorY+11392;
        motorJawsRotation = -motorX+11392;
        break;
    case Jaws:
        motorBase = motorZ + 11392;
        motorJawsRotation = -motorX+11392;
        motorJawsClench = 2*motorY+11392;
        break;
    case All:
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

    qDebug() << x << y << z << power << buttonFunction;
    qDebug() << motorBase << motorSegment2Middle << motorSegment1Bottom <<motorJawsClench << motorJawsRotation << motorJawsPosition;


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
