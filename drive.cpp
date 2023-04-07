#include "drive.h"

Drive::Drive(QObject *parent) : QObject(parent)
{

}

QByteArray Drive::calculateWheelsSpeeds(int x, int y, int power)
{

    // Calculate the normalized joystick values within the deadzone
    normX = qBound(-1.0, (double)x / MAX_JOYSTICK_VALUE, 1.0);
    normY = qBound(-1.0, (double)y / MAX_JOYSTICK_VALUE, 1.0);
    qDebug()<<normX<<normY;
    if (qAbs(x) < DEADZONE / MAX_JOYSTICK_VALUE && qAbs(y) < DEADZONE ) {
        // If the joystick values are within the deadzone, set the wheel speeds to 0
        leftSpeed = 3200;
        rightSpeed = 3200;
    }
    else {
        // Calculate the wheel speeds based on the differential drive system
        v = normY * MAX_SPEED *(power/50.0f);
        w = normX * MAX_SPEED * WHEELBASE / 2.0 *(power/50.0f);

        rightSpeed = qBound(0, static_cast<int>((v - w) * 3200 / MAX_SPEED + 3200), 6400);
        leftSpeed = qBound(0, static_cast<int>((v + w) * 3200 / MAX_SPEED + 3200), 6400);
    }

    // Pack the wheel speeds into a QByteArray
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream << (qint16)rightSpeed << (qint16)rightSpeed << (qint16)rightSpeed
           << (qint16)leftSpeed << (qint16)leftSpeed << (qint16)leftSpeed;

    qDebug() << leftSpeed << rightSpeed;

    return frame;
}

int16_t Drive::getLeftSpeed() const
{
return leftSpeed;
}

int16_t Drive::getRightSpeed() const
{
return rightSpeed;
}

