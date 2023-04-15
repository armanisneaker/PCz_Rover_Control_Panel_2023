#include "drive.h"

Drive::Drive(QObject *parent) : QObject(parent)
{
    frame.QByteArray::clear();
    leftSpeed = 3200;
    rightSpeed = 3200;
    containerX = 32767;
    containerY = 32767;
    containerPower = 0;
}

bool Drive::calculateWheelsSpeeds(const int x, const int y, const int power)
{
    //qDebug() << x << y;
    frame.QByteArray::clear();

    // Adjust the input range from 0 to 65535 to -32768 to 32767
    adjustedX = (x - 32768) /2 ;
    adjustedY = (y - 32768) /2;

    // Calculate the normalized joystick values within the deadzone
    normX = qBound(-1.0, (double)adjustedX / MAX_JOYSTICK_VALUE, 1.0);
    normY = qBound(-1.0, (double)adjustedY / MAX_JOYSTICK_VALUE, 1.0);

    if (qAbs(adjustedX) < DEADZONE && qAbs(adjustedY) < DEADZONE) {
        // If the joystick values are within the deadzone, set the wheel speeds to 0
        leftSpeed = 3200;
        rightSpeed = 3200;
    }
    else {
        // Calculate the wheel speeds based on the differential drive system
        v = normY * MAX_SPEED * (power / 50.0f);
        w = normX * MAX_SPEED * WHEELBASE / 2.0 * (power / 50.0f);

        rightSpeed = qBound(0, static_cast<int>((v - w) * 3200 / MAX_SPEED + 3200), 6400);
        leftSpeed = qBound(0, static_cast<int>((v + w) * 3200 / MAX_SPEED + 3200), 6400);
    }

    // Pack the wheel speeds into a QByteArray
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream << (qint16)rightSpeed << (qint16)rightSpeed << (qint16)rightSpeed
           << (qint16)leftSpeed << (qint16)leftSpeed << (qint16)leftSpeed;

    return true;
}

int16_t Drive::getLeftSpeed() const
{
    return leftSpeed;
}

int16_t Drive::getRightSpeed() const
{
    return rightSpeed;
}
