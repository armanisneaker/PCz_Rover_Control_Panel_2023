#include "drive.h"

Drive::Drive(QObject *parent) : QObject(parent)
{
    frame.QByteArray::clear();
    leftSpeed = 3200;
    rightSpeed = 3200;
    containerX = 32767;
    containerY = 32767;
    containerPower = 0;

    wheelDiameter = 0.035;
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
        if (leftSpeed != 3200) {
            leftSpeed = 3200;
            emit leftSpeedChanged(leftSpeed);
            emit roverSpeedChanged(0);
        }
        if (rightSpeed != 3200) {
            rightSpeed = 3200;
            emit rightSpeedChanged(rightSpeed);
            emit roverSpeedChanged(0);
        }
    }
    else {
        // Calculate the wheel speeds based on the differential drive system
        v = normY * MAX_SPEED * (power / 50.0f);
        w = normX * MAX_SPEED * WHEELBASE / 2.0 * (power / 50.0f);

        int16_t newRightSpeed = qBound(0, static_cast<int>((v - w) * 3200 / MAX_SPEED + 3200), 6400);
        int16_t newLeftSpeed = qBound(0, static_cast<int>((v + w) * 3200 / MAX_SPEED + 3200), 6400);

        /*if(adjustedY < 0)
        {
            int16_t temp;
            temp = newLeftSpeed;
            newLeftSpeed = newRightSpeed;
            newRightSpeed = temp;
        }






        /*if (newRightSpeed != rightSpeed || newLeftSpeed != leftSpeed) {
            double wheelCircumference = M_PI * wheelDiameter;

            double actualRightSpeed = ((newRightSpeed - 3200) * MAX_SPEED) / 3200;
            double actualLeftSpeed = ((newLeftSpeed - 3200) * MAX_SPEED) / 3200;

            double avgWheelSpeed = (actualRightSpeed + actualLeftSpeed) / 2.0;
            double roverSpeedKmh = (avgWheelSpeed * wheelCircumference * 60) / (1000);

            emit roverSpeedChanged(abs(roverSpeedKmh));
        }*/


        // Emit signals if the left or right speed has changed
        if (newRightSpeed != rightSpeed) {
            rightSpeed = newRightSpeed;
            emit rightSpeedChanged(rightSpeed);
        }
        if (newLeftSpeed != leftSpeed) {
            leftSpeed = newLeftSpeed;
            emit leftSpeedChanged(leftSpeed);
        }


    }

    if(rightSpeed > 6300) rightSpeed = 6300;
    if(rightSpeed < 100) rightSpeed = 100;
    if(leftSpeed > 6300) leftSpeed = 6300;
    if(leftSpeed < 100) leftSpeed = 100;


    // Pack the wheel speeds into a QByteArray
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream << (qint16)rightSpeed << (qint16)rightSpeed << (qint16)rightSpeed
           << (qint16)leftSpeed << (qint16)leftSpeed << (qint16)leftSpeed;

    return true;
}


